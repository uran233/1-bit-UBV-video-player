import struct
import glob
import os
import subprocess
import sys
import shutil

# Parametry docelowe formatu UBV
TARGET_W = 640       # 640 dla VGA lub 800 dla WVGA
TARGET_H = 480       # 480
TARGET_FPS = 25
TEMP_DIR = "frames"

def main():
    if len(sys.argv) < 2:
        print("Użycie: py konwertuj.py nazwa_filmu.mp4")
        return

    input_video = sys.argv[1]
    output_name = os.path.splitext(input_video)[0] + ".ubv"

    print(f"--- 0. Przygotowanie formatu .ubv ---")
    print(f"Format docelowy: {TARGET_W}x{TARGET_H} @ {TARGET_FPS} FPS")

    if os.path.exists(TEMP_DIR):
        shutil.rmtree(TEMP_DIR)
    os.makedirs(TEMP_DIR)

    print(f"--- 1. Skalowanie, negatyw i konwersja przez FFmpeg ---")
    # Dodany filtr 'negate', który odwraca kolory na poziomie Pythona
    ffmpeg_cmd = [
        "ffmpeg", "-i", input_video,
        "-vf", f"scale={TARGET_W}:{TARGET_H},fps={TARGET_FPS},negate,format=monow",
        "-vsync", "0",
        f"{TEMP_DIR}/frame_%05d.raw"
    ]
    
    try:
        subprocess.run(ffmpeg_cmd, check=True)
    except FileNotFoundError:
        print("BŁĄD: Nie znaleziono FFmpeg w systemie!")
        return

    files = sorted(glob.glob(f"{TEMP_DIR}/*.raw"))
    if not files:
        print("BŁĄD: FFmpeg nie wygenerował żadnych klatek.")
        return

    num_frames = len(files)
    raw_size = os.path.getsize(files[0])

    print(f"--- 2. Pakowanie do pliku: {output_name} ---")
    print(f"Klatek: {num_frames} | Rozmiar klatki: {raw_size} bajtów")

    with open(output_name, "wb") as out_f:
        header = struct.pack(
            "<4sHHHI I 14s",
            b"MBPB", TARGET_W, TARGET_H, TARGET_FPS, num_frames, raw_size, b"\x00" * 14
        )
        out_f.write(header)

        for i, file_path in enumerate(files):
            with open(file_path, "rb") as raw_f:
                out_f.write(raw_f.read())
            
            if i % 100 == 0:
                print(f"Postęp: {i}/{num_frames} klatek...", end="\r")

    print(f"\n--- GOTOWE! Plik {output_name} został utworzony. ---")
    shutil.rmtree(TEMP_DIR)

if __name__ == "__main__":
    main()