# LunatedEpsilon

**LunatedEpsilon** is a minimal Windows utility for converting between `.m3u` and `.m3u8` playlist formats.

It is designed for precise, predictable playlist restructuring without metadata synthesis, unnecessary abstraction, or hidden behavior.

## Features

  * **Bidirectional Conversion**: Seamlessly move between `.m3u` and `.m3u8`.
  * **Path Injection**: Base path injection for `.m3u` → `.m3u8` conversions.
  * **Location Control**:
      * Keep original file paths.
      * Replace with a custom base path.
  * **Slash Normalization**: Automatically converts backslashes (`\`) to forward slashes (`/`).
  * **Theming**: Light, Dark, AMOLED, and System theme support.
  * **Portable**: Single executable with a centered, minimal interface; no installation required.

## Conversion Behavior

### 1\. `.m3u` → `.m3u8`

#### Output Structure:

``` 
#EXTM3U
#playlistname.m3u8
/full/path/to/file.mp3

```

#### Behavior:

  * Adds `#EXTM3U` header.
  * Adds `#[output filename].m3u8` comment.
  * Prepends user-defined base path.
  * Normalizes path separators to `/`.
  * *Note: Base path must be provided before conversion is enabled.*

### 2\. `.m3u8` → `.m3u`

#### Output Structure:

``` 
/full/path/to/file.mp3

```

#### Behavior:

  * Removes all comment lines.
  * Preserves or rewrites file paths based on selection.
  * Does **NOT** generate `#EXTINF` or any metadata synthesis.

## Why This Tool Exists

Certain media players and environments require specific absolute or relative file paths. **LunatedEpsilon** provides deterministic restructuring without altering media metadata or introducing artificial tags.

## Usage

1.  Launch `LunatedEpsilon.exe`.
2.  Select a `.m3u` or `.m3u8` file.
3.  Provide the required base path if prompted.
4.  Choose your output location.
5.  Click **Convert**.

## Requirements

### Executable Release

  * Windows 10 or later.
  * No Python installation required.

### Running from Source

  * Python 3.11+
  * `customtkinter` library.

<!-- end list -->

``` bash
pip install customtkinter

```

### Building from Source

1.  **Clone the repository:**
    
    ``` bash
    git clone https://github.com/<your-username>/LunatedEpsilon.git
    cd LunatedEpsilon
    
    ```

2.  **Set up virtual environment:**
    
    ``` bash
    python -m venv venv
    .\venv\Scripts\activate
    pip install customtkinter pyinstaller
    
    ```

3.  **Build the executable:**
    
    ``` bash
    pyinstaller --noconfirm --onefile --windowed --icon=LE2.ico --add-data "LE2.ico;." main.py
    
    ```
    
    The compiled executable will be located in the `dist/` folder.

## Project Structure

``` 
LunatedEpsilon/
├── main.py
├── converter.py
├── LE4.ico
├── README.md
└── venv/ (ignored)

```

## Design Principles & Limitations

### Principles

  * **Deterministic:** No silent transformations or implicit assumptions.
  * **Clean:** No artificial metadata generation or "magic" tags.
  * **Minimal:** Focused interface surface and controlled normalization.

### Limitations

  * **Windows Only:** Current release target.
  * **No Metadata:** Does not generate `#EXTINF` or parse audio tags.
  * **Strict Scope:** Focused exclusively on playlist path restructuring.

## License

**Version:** v1.0.0

**License:** [Unicense License](https://unlicense.org/)
