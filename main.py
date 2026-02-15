import customtkinter as ctk
from tkinter import filedialog, messagebox
from converter import convertPlaylist
import os
import sys
import ctypes
import threading
import ctypes



if sys.platform.startswith("win"):
    ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID("LunatedEpsilon")

ctk.set_default_color_theme("dark-blue")
ctk.set_widget_scaling(1.0)
ctk.set_window_scaling(1.0)


class App(ctk.CTk):

    def __init__(self):
        super().__init__()

        self.defaultFgColor = self.cget("fg_color")

        self.windowWidth = 760
        self.windowHeight = 560

        self.title("LunatedEpsilon")
        self.geometry(f"{self.windowWidth}x{self.windowHeight}")
        self.minsize(600, 450)

        if sys.platform.startswith("win"):
            try:
                if hasattr(sys, "_MEIPASS"):
                    iconPath = os.path.join(sys._MEIPASS, "LE2.ico")
                else:
                    iconPath = os.path.join(os.path.dirname(__file__), "LE2.ico")

                self.iconbitmap(iconPath)
            except Exception:
                pass


        self.filePath = None
        self.inputExt = None

        # Root grid for vertical centering
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self.grid_columnconfigure(0, weight=1)

        fontPath = os.path.join(os.path.dirname(__file__), "JetBrainsMono-Regular.ttf")
        if os.path.exists(fontPath):
            ctypes.windll.gdi32.AddFontResourceW(fontPath)

        self.customFont = ctk.CTkFont(
            family="JetBrains Mono",
            size=18
        )

        self.createWidgets()

        self.after(0, lambda: self.eval("tk::PlaceWindow . center"))

    # ================= UI =================

    def createWidgets(self):

        # Container frame centered
        self.container = ctk.CTkFrame(self, fg_color="transparent")
        self.container.grid(row=1, column=0)
        self.container.grid_columnconfigure(0, weight=1)

        # Theme dropdown (top-right fixed)
        self.themeFrame = ctk.CTkFrame(self, fg_color="transparent")
        self.themeFrame.place(relx=0.97, rely=0.03, anchor="ne")

        self.themeLabel = ctk.CTkLabel(
            self.themeFrame,
            text="Theme:",
            font=self.customFont
        )
        self.themeLabel.pack(side="left", padx=(0, 6))

        self.themeMenu = ctk.CTkOptionMenu(
            self.themeFrame,
            values=["System", "Light", "Dark", "AMOLED"],
            command=self.changeTheme,
            width=140
        )
        self.themeMenu.set("System")
        self.themeMenu.pack(side="left")

    # =======================================

        self.selectBtn = ctk.CTkButton(
            self.container,
            text="Select File",
            command=self.selectFile,
            font=self.customFont,
            height=32
        )
        self.selectBtn.grid(row=0, column=0, pady=10)

        self.fileLabel = ctk.CTkLabel(
            self.container,
            text="No file selected",
            font=self.customFont
        )
        self.fileLabel.grid(row=1, column=0, pady=5)

        # Base Path Frame
        self.basePathFrame = ctk.CTkFrame(self.container)
        self.basePathFrame.grid(row=2, column=0, pady=8)
        self.basePathFrame.grid_remove()

        self.basePathEntry = ctk.CTkEntry(
            self.basePathFrame,
            placeholder_text="Base folder path",
            width=260,
            height=28
        )
        
        self.basePathEntry.bind("<KeyRelease>", lambda e: self.updateConvertState())

        self.basePathEntry.grid(row=0, column=0, padx=5)

        self.browseBtn = ctk.CTkButton(
            self.basePathFrame,
            text="Browse",
            width=80,
            height=28,
            fg_color="transparent",
            border_width=1,
            border_color=("#cccccc", "#444444"),
            hover_color=("#e6e6e6", "#333333"),
            text_color=("black", "white"),
            command=self.browseFolder
        )
        self.browseBtn.grid(row=0, column=1)

        # Location Mode
        self.locationModeOption = ctk.CTkOptionMenu(
            self.container,
            values=["Keep original path", "Use custom base path"],
            command=self.toggleLocationMode,
            width=220
        )
        self.locationModeOption.grid(row=3, column=0, pady=6)
        self.locationModeOption.grid_remove()

        self.customExtPathFrame = ctk.CTkFrame(self.container)
        self.customExtPathFrame.grid(row=4, column=0, pady=6)
        self.customExtPathFrame.grid_remove()

        self.customExtPathEntry = ctk.CTkEntry(
            self.customExtPathFrame,
            placeholder_text="Custom base path",
            width=260,
            height=28
        )
        self.customExtPathEntry.grid(row=0, column=0, padx=5)

        self.customExtBrowseBtn = ctk.CTkButton(
            self.customExtPathFrame,
            text="Browse",
            width=80,
            height=28,
            command=self.browseExtFolder
        )
        self.customExtBrowseBtn.grid(row=0, column=1)

        self.convertBtn = ctk.CTkButton(
            self.container,
            text="Convert",
            command=self.convert,
            state="disabled",
            font=self.customFont,
            height=34
        )
        self.convertBtn.grid(row=5, column=0, pady=12)

        self.progressBar = ctk.CTkProgressBar(self.container, width=300)
        self.progressBar.set(0)
        self.progressBar.grid(row=6, column=0, pady=5)
        self.progressBar.grid_remove()

        self.statusText = ctk.CTkLabel(
            self.container,
            text="",
            font=self.customFont
        )
        self.statusText.grid(row=7, column=0)
        self.statusText.grid_remove()

    # ================= Theme =================

    def changeTheme(self, choice):

        if choice == "AMOLED":
            ctk.set_appearance_mode("dark")
            self._setAppearanceOverride("#000000")
            return

        # All other modes
        self._clearAppearanceOverride()

        if choice == "Light":
            ctk.set_appearance_mode("light")

        elif choice == "Dark":
            ctk.set_appearance_mode("dark")

        elif choice == "System":
            ctk.set_appearance_mode("system")

    # ================= File =================

    def selectFile(self):

        path = filedialog.askopenfilename(
            filetypes=[("Playlist Files", "*.m3u *.m3u8")]
        )

        if not path:
            return

        self.filePath = path
        self.inputExt = os.path.splitext(path)[1].lower()

        self.fileLabel.configure(text=os.path.basename(path))

        if self.inputExt == ".m3u":
            self.convertBtn.configure(text="Convert to .m3u8")
            self.updateConvertState()

            self.basePathFrame.grid()
            self.locationModeOption.grid_remove()
            self.customExtPathFrame.grid_remove()

        elif self.inputExt == ".m3u8":
            self.convertBtn.configure(text="Convert to .m3u", state="normal")
            self.basePathFrame.grid_remove()
            self.locationModeOption.grid()
            self.customExtPathFrame.grid_remove()

    def toggleLocationMode(self, choice):

        if choice == "Use custom base path":
            self.customExtPathFrame.grid()
        else:
            self.customExtPathFrame.grid_remove()

    def browseFolder(self):
        folder = filedialog.askdirectory()
        if folder:
            self.basePathEntry.delete(0, "end")
            self.basePathEntry.insert(0, folder)

    def browseExtFolder(self):
        folder = filedialog.askdirectory()
        if folder:
            self.customExtPathEntry.delete(0, "end")
            self.customExtPathEntry.insert(0, folder)

    # ================= Conversion =================

    def convert(self):

        if not self.filePath:
            return

        targetExt = ".m3u8" if self.inputExt == ".m3u" else ".m3u"

        self.convertBtn.configure(state="disabled")

        savePath = filedialog.asksaveasfilename(
            defaultextension=targetExt,
            filetypes=[(targetExt.upper(), f"*{targetExt}")]
        )

        if not savePath:
            self.convertBtn.configure(state="normal")
            return

        basePath = None
        locationMode = "keep"

        if self.inputExt == ".m3u":
            basePath = self.basePathEntry.get().strip()
        else:
            choice = self.locationModeOption.get()
            if choice == "Use custom base path":
                locationMode = "custom"
                basePath = self.customExtPathEntry.get().strip()
                if not basePath:
                    return


        self.progressBar.grid()
        self.statusText.grid()
        self.statusText.configure(text="Processing")
        self.progressBar.set(0)
        self.convertBtn.configure(state="disabled")

        threading.Thread(
            target=self.performConversion,
            args=(savePath, basePath, locationMode),
            daemon=True
        ).start()

    def performConversion(self, savePath, basePath, locationMode):

        try:
            convertPlaylist(
                self.filePath,
                savePath,
                basePath=basePath,
                locationMode=locationMode
            )
            self.after(0, self.animateProgress)

        except Exception as e:
            self.after(0, lambda: messagebox.showerror("Error", str(e)))
            self.after(0, self.resetUi)

    # ================= Progress =================

    def animateProgress(self):

        current = self.progressBar.get()

        if current < 1:
            self.progressBar.set(current + 0.05)
            self.after(25, self.animateProgress)
        else:
            self.statusText.configure(text="Completed")
            self.after(900, self.fadeOut)

    def fadeOut(self):
        self.progressBar.grid_remove()
        self.statusText.grid_remove()
        self.progressBar.set(0)
        self.convertBtn.configure(state="normal")

    def resetUi(self):
        self.progressBar.grid_remove()
        self.statusText.grid_remove()
        self.progressBar.set(0)
        self.convertBtn.configure(state="normal")

    # ================== Helpers ===================

    def _setAppearanceOverride(self, color):
        self.configure(fg_color=color)

    def _clearAppearanceOverride(self):
        # force CTk to redraw with default theme color
        self.configure(fg_color=ctk.ThemeManager.theme["CTk"]["fg_color"])

    def updateConvertState(self):

        if not self.filePath:
            self.convertBtn.configure(state="disabled")
            return

        if self.inputExt == ".m3u":
            basePath = self.basePathEntry.get().strip()
            if basePath:
                self.convertBtn.configure(state="normal")
            else:
                self.convertBtn.configure(state="disabled")
        else:
            self.convertBtn.configure(state="normal")


if __name__ == "__main__":
    app = App()
    app.mainloop()
