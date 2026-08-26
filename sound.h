#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <thread>
#include <atomic>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>

class Sound_engine {
public:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int BUFFER_SIZE = 2048;
    static constexpr int NUM_BUFFERS = 3;

    Sound_engine() { initOpenAL(); }
    ~Sound_engine() { stop(); cleanupOpenAL(); }

    void start() {
        if (isRunning.load()) return;
        isRunning.store(true);
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            std::vector<short> tempBuffer(BUFFER_SIZE, 0);
            alBufferData(buffers[i], AL_FORMAT_MONO16, tempBuffer.data(), BUFFER_SIZE * sizeof(short), SAMPLE_RATE);
            alSourceQueueBuffers(source, 1, &buffers[i]);
        }
        alSourcePlay(source);
        audioThread = std::thread(&Sound_engine::audioLoop, this);
    }

    void stop() {
        if (!isRunning.load()) return;
        isRunning.store(false);
        if (audioThread.joinable()) audioThread.join();
        alSourceStop(source);
        ALint queued;
        alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
        while (queued > 0) {
            ALuint buffer;
            alSourceUnqueueBuffers(source, 1, &buffer);
            queued--;
        }
    }

    void setRPM(float newRpm) { rpm.store(std::clamp(newRpm, 600.0f, 7000.0f)); }
    void setThrottle(float newThrottle) { throttle.store(std::clamp(newThrottle, 0.0f, 1.0f)); }

private:
    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;
    ALuint source = 0;
    ALuint buffers[NUM_BUFFERS] = {0};
    std::thread audioThread;
    std::atomic<bool> isRunning{false};
    std::atomic<float> rpm{600.0f};
    std::atomic<float> throttle{0.0f};

    float phase = 0.0f;         
    float subPhase = 0.0f;      
    float subSubPhase = 0.0f;   
    float rumblePhase = 0.0f;   

    float lpState1 = 0.0f, lpState2 = 0.0f;
    float noiseLP1 = 0.0f, noiseLP2 = 0.0f;
    float jitterTarget = 1.0f;
    float jitterCurrent = 1.0f;
    float v8WobblePhase = 0.0f; 

    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> noiseDist{-1.0f, 1.0f};
    constexpr static float PI = 3.14159265359f;
    constexpr static float TWO_PI = 6.28318530718f;

    void initOpenAL() {
        device = alcOpenDevice(nullptr);
        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
        alGenSources(1, &source);
        alGenBuffers(NUM_BUFFERS, buffers);
        alSourcef(source, AL_GAIN, 1.0f);
        alSourcei(source, AL_LOOPING, AL_FALSE);
    }

    void cleanupOpenAL() {
        stop();
        if (source) alDeleteSources(1, &source);
        if (buffers[0]) alDeleteBuffers(NUM_BUFFERS, buffers);
        if (context) { alcMakeContextCurrent(nullptr); alcDestroyContext(context); }
        if (device) alcCloseDevice(device);
    }

    void audioLoop() {
        std::vector<short> tempBuffer(BUFFER_SIZE);
        while (isRunning.load()) {
            ALint processed;
            alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
            while (processed > 0) {
                ALuint buffer;
                alSourceUnqueueBuffers(source, 1, &buffer);
                generateSamples(tempBuffer.data(), BUFFER_SIZE);
                alBufferData(buffer, AL_FORMAT_MONO16, tempBuffer.data(), BUFFER_SIZE * sizeof(short), SAMPLE_RATE);
                alSourceQueueBuffers(source, 1, &buffer);
                processed--;
            }
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) alSourcePlay(source);
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); 
        }
    }

  void generateSamples(short* buffer, int numSamples) {
        float currentRPM = rpm.load();
        float currentThrottle = throttle.load();
        float rpmNorm = currentRPM / 7000.0f;

        float baseFreq = std::clamp((currentRPM / 60.0f) * 4.0f, 40.0f, 466.0f);
        
        float phaseInc = baseFreq / SAMPLE_RATE;
        float subPhaseInc = (baseFreq * 0.5f) / SAMPLE_RATE;
        
        float subSubPhaseInc = (baseFreq * 0.75f) / SAMPLE_RATE; 
        float rumblePhaseInc = (baseFreq * 2.0f) / SAMPLE_RATE;
        float wobblePhaseInc = (baseFreq * 0.5f) / SAMPLE_RATE; 

        float filterQ = 0.75f + (currentThrottle * 0.4f); 
        float cutoff = 0.04f + (currentThrottle * 0.08f) + (rpmNorm * 0.06f); 
        cutoff = std::clamp(cutoff, 0.03f, 0.35f);

        for (int i = 0; i < numSamples; ++i) {
            if (i % 128 == 0) {
                jitterTarget = 1.0f + (noiseDist(rng) - 0.5f) * 0.02f; 
            }
            jitterCurrent += (jitterTarget - jitterCurrent) * 0.1f;
            float currentPhaseInc = phaseInc * jitterCurrent;

            float phasePos = phase; // 0.0 to 1.0
            float wave = 0.0f;

            wave += std::sin(TWO_PI * phase) * 1.0f;
            wave += std::sin(TWO_PI * phase * 2.0f) * 0.6f;
            wave += std::sin(TWO_PI * phase * 3.0f) * 0.25f;

            float combustionSpike = std::max(0.0f, 1.0f - (phasePos * 5.0f));
            combustionSpike = combustionSpike * combustionSpike * combustionSpike; 
            
            float thud = std::sin(TWO_PI * phase * 1.5f) * combustionSpike * 0.9f * (0.6f + currentThrottle);
            float spikeNoise = noiseDist(rng) * combustionSpike * 0.25f * (0.5f + currentThrottle);
            wave += thud + spikeNoise;

            float bassBoost = 1.2f + (currentThrottle * 0.5f);
            if (rpmNorm < 0.5f) bassBoost *= 1.6f; 

            float subSubAmp = 0.6f * bassBoost;
            float subAmp = 0.7f * bassBoost;

            float lowEnd = (std::sin(TWO_PI * subPhase) * subAmp) + 
                           (std::sin(TWO_PI * subSubPhase) * subSubAmp);

            lowEnd = std::tanh(lowEnd * 3.5f) * 0.75f; 
            wave += lowEnd;

            v8WobblePhase += wobblePhaseInc;
            if (v8WobblePhase >= 1.0f) v8WobblePhase -= 1.0f;
            
            float v8Wobble = 1.0f;
            if (rpmNorm < 0.45f) {
                v8Wobble = 0.85f + 0.15f * std::sin(TWO_PI * v8WobblePhase);
            }
            wave *= v8Wobble;

            float exhaustNoiseEnv = std::max(0.0f, (phasePos - 0.2f) * 1.25f);
            float rawNoise = noiseDist(rng);
            noiseLP1 = noiseLP1 + 0.08f * (rawNoise - noiseLP1);
            noiseLP2 = noiseLP2 + 0.04f * (noiseLP1 - noiseLP2);
            wave += noiseLP2 * exhaustNoiseEnv * 0.2f * currentThrottle;

            for (int stage = 0; stage < 2; ++stage) {
                float feedback = filterQ * (1.0f - cutoff);
                wave = wave + feedback * lpState1;
                wave = std::tanh(wave * 1.2f); // Ограничитель внутри фильтра предотвращает "вой"
                lpState2 = lpState1;
                lpState1 = wave;
                wave = wave * cutoff + lpState1 * (1.0f - cutoff);
            }
            
            wave *= 1.6f; 

            float volume = 0.45f + 0.55f * currentThrottle;
            wave *= volume;

            wave = std::tanh(wave * 2.0f);

            buffer[i] = static_cast<short>(std::clamp(wave * 32000.0f, -32768.0f, 32767.0f));

            phase += currentPhaseInc; if (phase >= 1.0f) phase -= 1.0f;
            subPhase += subPhaseInc; if (subPhase >= 1.0f) subPhase -= 1.0f;
            subSubPhase += subSubPhaseInc; if (subSubPhase >= 1.0f) subSubPhase -= 1.0f;
            rumblePhase += rumblePhaseInc; if (rumblePhase >= 1.0f) rumblePhase -= 1.0f;
        }
    }
};
