#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <thread>
#include <atomic>
#include <regex>
#include <mutex>
#define MA_IMPLEMENTATION
#include "lib/miniaudio.h"

using namespace std;

// Структура для хранения ноты
struct Note {
    double frequency;
    double duration;
    double amplitude;  // Изменили atomic<double> на обычный double
    bool fadeOut;

    // Добавили конструктор для инициализации
    Note(double freq, double dur, double amp, bool fade)
            : frequency(freq), duration(dur), amplitude(amp), fadeOut(fade) {}
};

vector<Note> activeNotes;
atomic<bool> stopPlayback(false);  // Добавьте эту строку
mutex notesMutex;
ma_device device;

// Частоты нот для октавы 4 (A4 = 440Hz)
map<string, double> noteFrequencies = {
        {"C", 261.63}, {"C#", 277.18},
        {"D", 293.66}, {"D#", 311.13},
        {"E", 329.63},
        {"F", 349.23}, {"F#", 369.99},
        {"G", 392.00}, {"G#", 415.30},
        {"A", 440.00}, {"A#", 466.16},
        {"B", 493.88}
};

// Функция расчета частоты ноты
double calculateFrequency(const string& noteStr) {
    regex pattern(R"(([A-G])(#)?([0-8]))");
    smatch matches;

    if (regex_match(noteStr, matches, pattern)) {
        string note = matches[1];
        string sharp = matches[2];
        int octave = stoi(matches[3]);

        string key = note + sharp;
        if (noteFrequencies.find(key) == noteFrequencies.end()) return 0.0;

        double baseFrequency = noteFrequencies[key];
        return baseFrequency * pow(2, octave - 4);
    }
    return 0.0;
}

// Audio callback функция
void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    float* output = (float*)pOutput;
    double sampleRate = device.sampleRate;

    lock_guard<mutex> lock(notesMutex);  // Защищаем доступ к activeNotes

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        output[i] = 0.0f;

        for (auto& note : activeNotes) {
            static double phase = 0.0;
            double sample = note.amplitude * sin(phase);

            if (note.fadeOut) {
                note.amplitude = max(0.0, note.amplitude - (1.0 / (note.duration * sampleRate)));
            }

            output[i] += sample;
            phase += 2.0 * M_PI * note.frequency / sampleRate;

            if (phase >= 2.0 * M_PI) {
                phase -= 2.0 * M_PI;
            }
        }

        // Удаление завершившихся нот
        activeNotes.erase(
                remove_if(activeNotes.begin(), activeNotes.end(),
                          [](const Note& n) { return n.amplitude <= 0.0; }),
                activeNotes.end());
    }
}
// Функция чтения музыкального файла
vector<vector<string>> readMusicFile(const string& filename, int& bpm) {
    ifstream file(filename);
    vector<vector<string>> music;

    if (!file.is_open()) throw runtime_error("Не удалось открыть файл");

    string line;
    getline(file, line);
    bpm = stoi(line);

    while (getline(file, line)) {
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        if (line.empty() || line == "END") break;
        music.push_back({line});
    }

    return music;
}

// Функция воспроизведения
void playMusic(const vector<vector<string>>& music, int bpm) {
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate        = 44100;
    deviceConfig.dataCallback      = audioCallback;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        cerr << "Ошибка инициализации аудиоустройства" << endl;
        return;
    }

    ma_device_start(&device);

    double lineDuration = 60.0 / bpm;

    for (const auto& line : music) {
        if (stopPlayback) break;

        {
            lock_guard<mutex> lock(notesMutex);  // Защищаем доступ при добавлении нот
            for (const auto& noteStr : line) {
                if (noteStr == "-") {
                    for (auto& note : activeNotes) {
                        note.fadeOut = true;
                    }
                }
                else {
                    double freq = calculateFrequency(noteStr);
                    if (freq > 0) {
                        activeNotes.emplace_back(freq, lineDuration, 1.0, false);
                    }
                }
            }
        }

        this_thread::sleep_for(chrono::duration<double>(lineDuration));
    }

    // Ждем завершения всех нот
    while (!activeNotes.empty()) {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    ma_device_uninit(&device);
}

int main() {
    try {
        string filename;
        cout << "Введите имя файла с нотами: ";
        cin >> filename;

        int bpm;
        auto music = readMusicFile(filename, bpm);

        cout << "Воспроизведение начато. Нажмите Enter для остановки...";
        thread playbackThread(playMusic, music, bpm);

        cin.ignore();  // Добавьте эту строку
        cin.get();     // Ожидание нажатия Enter

        stopPlayback = true;
        playbackThread.join();

        cout << "Воспроизведение завершено." << endl;
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}