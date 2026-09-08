#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern void process_frame_asm(void* pixel_data, void* texture_buffer, int width, int height);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uzycie: player.exe nazwa_filmu.ubv\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("Blad: Nie moge otworzyc pliku!\n");
        return 1;
    }

    uint8_t ubv_header[32];
    if (fread(ubv_header, 1, 32, file) < 32) {
        printf("Blad: Plik naglowkowy jest uszkodzony!\n");
        fclose(file);
        return 1;
    }

    uint16_t width      = *(uint16_t*)&ubv_header[4];
    uint16_t height     = *(uint16_t*)&ubv_header[6];
    uint16_t fps        = *(uint16_t*)&ubv_header[8];
    uint32_t num_frames = *(uint32_t*)&ubv_header[10];
    uint32_t raw_size   = *(uint32_t*)&ubv_header[14];

    if (fps == 0) fps = 25; 

    printf("Format UBV: %dx%d, %d FPS, Klatek: %u, Rozmiar klatki: %u bajtow\n", 
           width, height, fps, num_frames, raw_size);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        fclose(file);
        return 1;
    }

    // --- SEKCJA INICJALIZACJI Z OBSŁUGĄ SKALOWANIA ---
    
    // Zezwalamy na zmianę rozmiaru okna (SDL_WINDOW_RESIZABLE)
    SDL_Window* window = SDL_CreateWindow("Uranko Bitmap Video Player", width, height, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    
    // SDL automatycznie dodaje czarne pasy, żeby zachować proporcje filmu
    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    
    // Wymuszamy ostre skalowanie, bez rozmywania pikseli
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    
    // -------------------------------------------------

    uint8_t* frame_full = malloc(raw_size);
    if (!frame_full) {
        printf("Blad: Brak pamieci!\n");
        fclose(file);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    int paused = 0;
    SDL_Event event;
    uint32_t frame_count = 0;

    long seek_offset = 5 * fps * raw_size;
    uint32_t delay_ms = 1000 / fps; // Automatyczne obliczanie milisekund z FPS

    while (running) {
        // --- OBSŁUGA ZDARZEŃ I KLAWIATURY ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            } 
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    case SDL_SCANCODE_SPACE:
                        paused = !paused;
                        if (paused) {
                            SDL_SetWindowTitle(window, "Uranko Bitmap Video Player [PAUZA]");
                        } else {
                            SDL_SetWindowTitle(window, "Uranko Bitmap Video Player");
                        }
                        break;

                    case SDL_SCANCODE_RIGHT:
                        fseek(file, seek_offset, SEEK_CUR);
                        frame_count += 5 * fps;
                        if (frame_count >= num_frames) frame_count = num_frames - 1;
                        break;

                    case SDL_SCANCODE_LEFT:
                        {
                            long current_pos = ftell(file);
                            long target_pos = current_pos - seek_offset - raw_size;
                            if (target_pos < 32) target_pos = 32;
                            fseek(file, target_pos, SEEK_SET);
                            
                            if (frame_count > 5 * fps) {
                                frame_count -= 5 * fps;
                            } else {
                                frame_count = 0;
                            }
                        }
                        break;

                    default:
                        break;
                }
            }
        }

        if (frame_count >= num_frames) {
            SDL_SetWindowTitle(window, "Uranko Bitmap Video Player [KONIEC]");
            SDL_Delay(33);
            continue;
        }

        // --- GŁÓWNA LOGIKA ODTWARZANIA ---
        if (!paused) {
            size_t read = fread(frame_full, 1, raw_size, file);
            if (read < raw_size) {
                frame_count = num_frames;
                continue;
            }

            void* tex_pixels;
            int pitch;
            if (SDL_LockTexture(texture, NULL, &tex_pixels, &pitch)) {
                // Wywołanie Twojego ASM - bez zmian
                process_frame_asm(frame_full, tex_pixels, (int)width, (int)height);
                SDL_UnlockTexture(texture);
            }

            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            
            frame_count++;
        }

        SDL_Delay(delay_ms); 
    }

    printf("Odtwarzacz zamkniety.\n");
    free(frame_full);
    fclose(file);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}