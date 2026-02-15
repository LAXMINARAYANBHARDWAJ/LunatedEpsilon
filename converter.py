import os


def convertPlaylist(
    inputPath,
    outputPath,
    basePath=None,
    locationMode="keep"  # keep | custom
):

    if not inputPath.lower().endswith((".m3u", ".m3u8")):
        raise ValueError("Unsupported file type")

    inputExt = os.path.splitext(inputPath)[1].lower()

    if inputExt == ".m3u":
        if not basePath:
            raise ValueError("Base path required for M3U → M3U8 conversion")
        _convertM3uToM3u8(inputPath, outputPath, basePath)

    elif inputExt == ".m3u8":
        _convertM3u8ToM3u(inputPath, outputPath, locationMode, basePath)


# ================= M3U to M3U8 =================

def _convertM3uToM3u8(inputPath, outputPath, basePath):

    basePath = basePath.rstrip("/\\").replace("\\", "/")

    outputName = os.path.splitext(os.path.basename(outputPath))[0]

    with open(inputPath, "r", encoding="utf-8", errors="ignore") as infile, \
         open(outputPath, "w", encoding="utf-8") as outfile:

        outfile.write("#EXTM3U\n")
        outfile.write(f"#{outputName}.m3u8\n")

        for line in infile:
            line = line.strip()

            if not line or line.startswith("#"):
                continue

            if line.startswith("Music/"):
                line = line[len("Music/"):]

            line = line.replace("\\", "/")

            fullPath = f"{basePath}/{line}"
            outfile.write(fullPath + "\n")

# ================= M3U8 to M3U =================

def _convertM3u8ToM3u(inputPath, outputPath, locationMode, customBase):

    customBaseNormalized = None

    if locationMode == "custom":
        if not customBase:
            raise ValueError("Custom base path required")
        customBaseNormalized = customBase.rstrip("/\\").replace("\\", "/")

    with open(inputPath, "r", encoding="utf-8", errors="ignore") as infile, \
         open(outputPath, "w", encoding="utf-8") as outfile:

        for line in infile:
            stripped = line.strip()

            if not stripped or stripped.startswith("#"):
                continue

            originalPath = stripped.replace("\\", "/")
            filename = os.path.basename(originalPath)

            if locationMode == "keep":
                outfile.write(originalPath + "\n")

            elif locationMode == "custom":
                newPath = f"{customBaseNormalized}/{filename}"
                outfile.write(newPath + "\n")
