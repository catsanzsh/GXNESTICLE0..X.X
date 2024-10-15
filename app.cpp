#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <SDL2/SDL.h>

// Constants for NES
const int NES_WIDTH = 256;
const int NES_HEIGHT = 240;

// CPU Registers
struct CPU6502 {
    uint8_t A, X, Y;  // Accumulator, Index X, Index Y
    uint8_t SP;       // Stack Pointer
    uint16_t PC;      // Program Counter
    uint8_t status;   // Status Register

    void reset() {
        A = X = Y = 0;
        SP = 0xFD;
        PC = 0x8000;  // Start address for NES programs
        status = 0x24;
    }

    void execute_opcode(uint8_t opcode, Memory& memory) {
        switch (opcode) {
            case 0xA9:  // LDA Immediate
                A = memory.read(PC++);
                update_zero_and_negative_flags(A);
                break;
            case 0xAA:  // TAX
                X = A;
                update_zero_and_negative_flags(X);
                break;
            case 0xE8:  // INX
                X++;
                update_zero_and_negative_flags(X);
                break;
            // Add more opcodes as needed
            default:
                std::cerr << "Unknown opcode: " << std::hex << static_cast<int>(opcode) << std::endl;
                break;
        }
    }

    void update_zero_and_negative_flags(uint8_t value) {
        if (value == 0) {
            status |= 0x02;  // Set zero flag
        } else {
            status &= ~0x02;  // Clear zero flag
        }

        if (value & 0x80) {
            status |= 0x80;  // Set negative flag
        } else {
            status &= ~0x80;  // Clear negative flag
        }
    }
};

// Memory
class Memory {
public:
    std::vector<uint8_t> ram;

    Memory() : ram(0x10000, 0) {}  // NES has 64KB addressable memory

    uint8_t read(uint16_t address) const {
        return ram[address];
    }

    void write(uint16_t address, uint8_t value) {
        ram[address] = value;
    }
};

// PPU placeholder
class PPU {
public:
    void render_frame(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // Render frame logic here
        SDL_RenderPresent(renderer);
    }
};

// NES Emulator Class
class NesticleClone {
public:
    CPU6502 cpu;
    Memory memory;
    PPU ppu;
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;

    NesticleClone() : running(true), window(nullptr), renderer(nullptr) {}

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow("NesticleClone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, NES_WIDTH * 2, NES_HEIGHT * 2, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
            return false;
        }

        cpu.reset();
        return true;
    }

    void load_rom(const std::string& filename) {
        std::ifstream rom(filename, std::ios::binary);
        if (!rom) {
            std::cerr << "Failed to open ROM file: " << filename << std::endl;
            return;
        }

        rom.seekg(0, std::ios::end);
        size_t rom_size = rom.tellg();
        rom.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(rom_size);
        rom.read(reinterpret_cast<char*>(buffer.data()), rom_size);
        for (size_t i = 0; i < buffer.size(); ++i) {
            memory.write(0x8000 + i, buffer[i]);  // Load ROM into memory starting at 0x8000
        }
    }

    void handle_input() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    void emulate_cycle() {
        uint8_t opcode = memory.read(cpu.PC);
        cpu.PC++;
        cpu.execute_opcode(opcode, memory);
    }

    void run() {
        while (running) {
            handle_input();
            emulate_cycle();
            ppu.render_frame(renderer);
            SDL_Delay(16);  // Roughly 60 FPS
        }
    }

    void cleanup() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    NesticleClone emulator;
    if (!emulator.init()) {
        return 1;
    }

    emulator.load_rom(argv[1]);
    emulator.run();
    emulator.cleanup();

    return 0;
}
