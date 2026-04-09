/*
 * ============================================================
 *  MAX30100_Sim.h  (v2 - fixed)
 *  Mô phỏng cảm biến MAX30100
 *  Pipeline:
 *    1. Sinh tín hiệu IR/RED thô (DC + AC sóng tim + nhiễu)
 *    2. Tách AC/DC → tính R → SpO2 = 110 - 25*R
 *    3. Đếm đỉnh IR → BPM
 *    4. Warm-up 3 giây + motion artifact
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include <math.h>

#define SIM_SAMPLE_RATE_MS   20      // 50Hz
#define SIM_WARMUP_MS        3000
#define SIM_BUFFER_SIZE      150     // 3 giây buffer (150 * 20ms)

// ── Tham số sinh lý mục tiêu ──────────────────────────────────
#define TARGET_BPM           75.0f   // nhịp tim bình thường
#define TARGET_SPO2          97.0f   // sinh lý bình thường

// ── Tham số tín hiệu ──────────────────────────────────────────
// Chọn DC và AC sao cho R = (AC_red/DC_red)/(AC_ir/DC_ir)
// dẫn đến SpO2 = 110 - 25*R ≈ 97%
// → R ≈ (110 - 97) / 25 = 0.52
// → AC_red/DC_red = 0.52 * AC_ir/DC_ir
// Chọn: AC_ir/DC_ir = 0.04 → AC_red/DC_red = 0.0208
#define DC_IR                50000.0f
#define AC_IR                2000.0f    // AC_ir/DC_ir = 0.04
#define DC_RED               45000.0f
#define AC_RED               936.0f     // AC_red/DC_red = 0.0208 → R=0.52 → SpO2≈97%

// ── Nhiễu ─────────────────────────────────────────────────────
#define NOISE_AMP            80.0f
#define MOTION_AMP           3000.0f
#define MOTION_PROB          0.02f      // 2% mỗi sample
#define MOTION_DURATION_MS   300

class MAX30100Sim {
public:

    MAX30100Sim() {
        _ready      = false;
        _startMs    = 0;
        _lastSample = 0;
        _bufferIdx  = 0;
        _bufferFull = false;
        _inMotion   = false;
        _motionEnd  = 0;
        _bpmOutput  = 0.0f;
        _spo2Output = 0.0f;
        _rawIR      = 0.0f;
        _rawRed     = 0.0f;
        // drift
        _bpmDrift   = 0.0f;
        _spo2Drift  = 0.0f;
        memset(_irBuffer,  0, sizeof(_irBuffer));
        memset(_redBuffer, 0, sizeof(_redBuffer));
    }

    bool begin() {
        _startMs    = millis();
        _lastSample = millis();
        randomSeed(analogRead(0));
        Serial.println("[MAX30100_Sim] Warm-up 3 giay...");
        return true;
    }

    void update() {
        unsigned long now = millis();

        // Warm-up
        if (!_ready) {
            if (now - _startMs >= SIM_WARMUP_MS) {
                _ready = true;
                // Khởi tạo output bằng giá trị mục tiêu
                _bpmOutput  = TARGET_BPM;
                _spo2Output = TARGET_SPO2;
                Serial.println("[MAX30100_Sim] San sang!");
            }
            return;
        }

        if (now - _lastSample < SIM_SAMPLE_RATE_MS) return;
        _lastSample = now;

        _updateDrift();
        _updateMotion(now);

        // Tần số góc theo BPM hiện tại
        float bpmNow = TARGET_BPM + _bpmDrift;
        float omega  = 2.0f * PI * (bpmNow / 60.0f);
        float t      = now / 1000.0f;

        // Dạng sóng PPG: sin chính + harmonic nhỏ
        float wave = sin(omega * t) + 0.15f * sin(2.0f * omega * t);

        // Nhiễu gaussian
        float noiseIR  = _gaussNoise(NOISE_AMP);
        float noiseRed = _gaussNoise(NOISE_AMP);
        float motion   = _inMotion ? _gaussNoise(MOTION_AMP) : 0.0f;

        // SpO2 drift ảnh hưởng nhẹ vào AC_RED
        float spo2Now   = TARGET_SPO2 + _spo2Drift;
        // R mục tiêu từ SpO2: R = (110 - spo2) / 25
        float R_target  = (110.0f - spo2Now) / 25.0f;
        // AC_red cần thiết: AC_red = R_target * (AC_ir/DC_ir) * DC_red
        float acRedNow  = R_target * (AC_IR / DC_IR) * DC_RED;

        _rawIR  = DC_IR  + AC_IR     * wave + noiseIR  + motion;
        _rawRed = DC_RED + acRedNow  * wave + noiseRed + motion;

        _rawIR  = max(_rawIR,  0.0f);
        _rawRed = max(_rawRed, 0.0f);

        // Đẩy vào buffer
        _irBuffer[_bufferIdx]  = _rawIR;
        _redBuffer[_bufferIdx] = _rawRed;
        _bufferIdx = (_bufferIdx + 1) % SIM_BUFFER_SIZE;
        if (_bufferIdx == 0) _bufferFull = true;

        if (_bufferFull) {
            _calculateSpO2();
            _calculateBPM();
        }
    }

    float getHeartRate()      { return _ready ? _bpmOutput  : 0.0f; }
    float getSpO2()           { return _ready ? _spo2Output : 0.0f; }
    bool  isReady()           { return _ready; }
    float getRawIR()          { return _rawIR;  }
    float getRawRed()         { return _rawRed; }
    float getWarmupProgress() {
        if (_ready) return 1.0f;
        return constrain((float)(millis() - _startMs) / SIM_WARMUP_MS, 0.0f, 1.0f);
    }

private:
    float         _rawIR, _rawRed;
    float         _irBuffer[SIM_BUFFER_SIZE];
    float         _redBuffer[SIM_BUFFER_SIZE];
    int           _bufferIdx;
    bool          _bufferFull;
    float         _bpmOutput, _spo2Output;
    float         _bpmDrift,  _spo2Drift;
    bool          _ready;
    unsigned long _startMs, _lastSample;
    bool          _inMotion;
    unsigned long _motionEnd;

    // ── Drift nhẹ theo thời gian ──────────────────────────────
    void _updateDrift() {
        if (random(100) < 3) {
            _bpmDrift  += (float)random(-5, 6) / 10.0f;    // ±0.5 BPM
            _spo2Drift += (float)random(-3, 4) / 100.0f;   // ±0.03%
        }
        _bpmDrift  = constrain(_bpmDrift,  -15.0f, 15.0f);
        _spo2Drift = constrain(_spo2Drift,  -4.0f,  2.0f);
    }

    // ── Motion artifact ───────────────────────────────────────
    void _updateMotion(unsigned long now) {
        if (_inMotion) {
            if (now >= _motionEnd) {
                _inMotion = false;
                Serial.println("[MAX30100_Sim] Het motion artifact");
            }
        } else {
            if ((float)random(10000) / 10000.0f < MOTION_PROB) {
                _inMotion  = true;
                _motionEnd = now + MOTION_DURATION_MS;
                Serial.println("[MAX30100_Sim] Motion artifact!");
            }
        }
    }

    // ── Tính SpO2 từ tỉ lệ AC/DC ─────────────────────────────
    void _calculateSpO2() {
        // DC = mean
        float dcIR = 0, dcRed = 0;
        for (int i = 0; i < SIM_BUFFER_SIZE; i++) {
            dcIR  += _irBuffer[i];
            dcRed += _redBuffer[i];
        }
        dcIR  /= SIM_BUFFER_SIZE;
        dcRed /= SIM_BUFFER_SIZE;

        if (dcIR < 1.0f || dcRed < 1.0f) return;

        // AC = RMS của phần dao động
        float acIR = 0, acRed = 0;
        for (int i = 0; i < SIM_BUFFER_SIZE; i++) {
            float dIR  = _irBuffer[i]  - dcIR;
            float dRed = _redBuffer[i] - dcRed;
            acIR  += dIR  * dIR;
            acRed += dRed * dRed;
        }
        acIR  = sqrt(acIR  / SIM_BUFFER_SIZE);
        acRed = sqrt(acRed / SIM_BUFFER_SIZE);

        if (acIR < 1.0f) return;

        float R        = (acRed / dcRed) / (acIR / dcIR);
        float rawSpO2  = 110.0f - 25.0f * R;
        rawSpO2        = constrain(rawSpO2, 80.0f, 100.0f);

        // Low-pass mạnh để ổn định
        _spo2Output = 0.85f * _spo2Output + 0.15f * rawSpO2;
    }

    // ── Tính BPM bằng đếm zero-crossing ──────────────────────
    void _calculateBPM() {
        float mean = 0;
        for (int i = 0; i < SIM_BUFFER_SIZE; i++) mean += _irBuffer[i];
        mean /= SIM_BUFFER_SIZE;

        // Ngưỡng hysteresis tránh đếm nhầm do nhiễu
        float threshUp   =  mean * 0.003f;
        float threshDown = -mean * 0.003f;
        int   crossings  = 0;
        bool  above      = false;

        for (int i = 0; i < SIM_BUFFER_SIZE; i++) {
            float ac = _irBuffer[i] - mean;
            if (!above && ac > threshUp) {
                above = true;
                crossings++;
            } else if (above && ac < threshDown) {
                above = false;
            }
        }

        // Buffer = 3 giây (SIM_BUFFER_SIZE * 20ms = 3000ms)
        // BPM = crossings / 3 * 60
        float rawBPM = (float)crossings * 20.0f;

        if (rawBPM >= 40.0f && rawBPM <= 180.0f) {
            _bpmOutput = 0.75f * _bpmOutput + 0.25f * rawBPM;
        }
    }

    // ── Gaussian noise xấp xỉ ────────────────────────────────
    float _gaussNoise(float amp) {
        float n = 0;
        for (int i = 0; i < 4; i++) n += (float)random(-1000, 1001);
        return (n / 4000.0f) * amp;
    }
};