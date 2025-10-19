# Pars Assistant

**Pars Assistant** is a simple and lightweight desktop assistant written in **C** with **GTK**.  
It uses the **Google Gemini API** to generate responses and aims to provide a minimal, fast, and native experience — without Python or heavy dependencies.

---

## 📂 Project Structure

pars/
├── main.c # Main C source code (GTK + CURL + JSON)
├── images/ # App icons and UI visuals
│ ├── logo.png
│ └── background.png
├── deb_builder.sh # Shell script to build the .deb package
└── README.md # You’re reading this file


---

## ⚙️ Requirements

You need the following packages to compile the project:

sudo apt install build-essential libgtk-3-dev libcurl4-openssl-dev libjson-c-dev

🧠 Features
Clean GTK-based interface

Uses Google Gemini API for responses

Includes app icons and visuals

Can be packaged into a .deb installer# 🐍 Pars Assistant

**Pars Assistant** is a simple and lightweight desktop assistant written in **C** with **GTK**.  
It uses the **Google Gemini API** to generate responses and aims to provide a minimal, fast, and native experience — without Python or heavy dependencies.

---

## 📂 Project Structure

pars/
├── main.c # Main C source code (GTK + CURL + JSON)
├── images/ # App icons and UI visuals
│ ├── logo.png
│ └── background.png
├── deb_builder.sh # Shell script to build the .deb package
└── README.md # You’re reading this file


---

## ⚙️ Requirements

You need the following packages to compile the project:


sudo apt install build-essential libgtk-3-dev libcurl4-openssl-dev libjson-c-dev

🧠 Features
Clean GTK-based interface

Uses Google Gemini API for responses

Includes app icons and visuals

Can be packaged into a .deb installer

Open-source and easy to modify

🚀 Build Instructions
To compile and run manually:

gcc main.c -o pars `pkg-config --cflags --libs gtk+-3.0 json-c libcurl`
./pars
To build a .deb package:

chmod +x deb_builder.sh
./deb_builder.sh
Your .deb package will be created in the pars_deb/ directory.

📦 Install the .deb Package
Once built, you can install it using:

sudo dpkg -i pars_1.0_amd64.deb
Then launch it from your applications menu.

🖼️ Visuals
All icons and images are included in the images/ directory.
If you modify them, just re-run deb_builder.sh to include the new files in your package.

🧰 Environment Variables
If you are using the Gemini API, make sure your API key is defined inside the source:

#define API_KEY "your_gemini_api_key"
Or you can load it dynamically from an environment variable later.

🧑‍💻 Contribution
Feel free to fork, modify, and submit pull requests.
All contributions are welcome.

📜 License
This project is open-source under the MIT License.

Author: Herdem Erdem
Version: 1.0
Platform: Linux (GTK3)

Open-source and easy to modify

🚀 Build Instructions
To compile and run manually:

gcc main.c -o pars `pkg-config --cflags --libs gtk+-3.0 json-c libcurl`
./pars
To build a .deb package:

chmod +x deb_builder.sh
./deb_builder.sh
Your .deb package will be created in the pars_deb/ directory.

📦 Install the .deb Package
Once built, you can install it using:

sudo dpkg -i pars_1.0_amd64.deb
Then launch it from your applications menu.

🖼️ Visuals
All icons and images are included in the images/ directory.
If you modify them, just re-run deb_builder.sh to include the new files in your package.

🧰 Environment Variables
If you are using the Gemini API, make sure your API key is defined inside the source:

#define API_KEY "your_gemini_api_key"
Or you can load it dynamically from an environment variable later.

🧑‍💻 Contribution
Feel free to fork, modify, and submit pull requests.
All contributions are welcome.

📜 License
This project is open-source under the MIT License.

Author: Hidayet Erdem
Version: 1.0
Platform: Linux (GTK3)
