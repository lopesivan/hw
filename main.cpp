// hw.cpp - C++17 CLI "hello world" templates
// Build: g++ -std=c++17 -O2 -Wall -Wextra hw.cpp -o hw
// Windows (MSVC): cl /std:c++17 hw.cpp

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// -------------------- util: trim/startsWith --------------------
static inline std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    return s;
}
static inline std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    return s;
}
static inline std::string trim(std::string s) { return rtrim(ltrim(std::move(s))); }
static inline bool starts_with(const std::string& s, const std::string& pref) {
    return s.size() >= pref.size() && std::equal(pref.begin(), pref.end(), s.begin());
}
static inline std::string to_lower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

// -------------------- OS-specific dirs --------------------
static fs::path user_config_dir() {
#ifdef _WIN32
    const char* app = std::getenv("APPDATA");
    if (app && *app) return fs::path(app) / "hw";
    return fs::path(std::getenv("USERPROFILE") ? std::getenv("USERPROFILE") : "") / "AppData" / "Roaming" / "hw";
#elif __APPLE__
    return fs::path(std::getenv("HOME") ? std::getenv("HOME") : "") / "Library" / "Application Support" / "hw";
#else
    return fs::path(std::getenv("HOME") ? std::getenv("HOME") : "") / ".config" / "hw";
#endif
}
static fs::path user_data_dir() {
#ifdef _WIN32
    const char* app = std::getenv("APPDATA");
    if (app && *app) return fs::path(app) / "hw";
    return fs::path(std::getenv("USERPROFILE") ? std::getenv("USERPROFILE") : "") / "AppData" / "Roaming" / "hw";
#elif __APPLE__
    return fs::path(std::getenv("HOME") ? std::getenv("HOME") : "") / "Library" / "Application Support" / "hw";
#else
    return fs::path(std::getenv("HOME") ? std::getenv("HOME") : "") / ".local" / "share" / "hw";
#endif
}

// -------------------- read/write helpers --------------------
static bool read_file(const fs::path& p, std::string& out) {
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs) return false;
    std::ostringstream oss;
    oss << ifs.rdbuf();
    out = oss.str();
    return true;
}
static bool write_file(const fs::path& p, const std::string& txt) {
    std::ofstream ofs(p, std::ios::binary);
    if (!ofs) return false;
    ofs << txt;
    return true;
}

// -------------------- config (INI minimal) --------------------
// Formato aceito:
// default_lang = cpp
//
// [aliases]
// c = c
// cpp = cpp
// rs = rust
struct Config {
    std::string default_lang = "c";
    std::unordered_map<std::string, std::string> aliases; // alias -> language
};

static Config load_config() {
    Config cfg;
    fs::path cfgPath = user_config_dir() / "config.ini";
    std::ifstream in(cfgPath);
    if (!in) {
        // ok — config é opcional
        return cfg;
    }
    std::string line;
    std::string section;
    while (std::getline(in, line)) {
        std::string raw = trim(line);
        if (raw.empty() || raw[0] == '#' || raw[0] == ';') continue;

        if (raw.front() == '[' && raw.back() == ']') {
            section = to_lower(raw.substr(1, raw.size()-2));
            continue;
        }

        auto pos = raw.find('=');
        if (pos == std::string::npos) continue;
        std::string key = to_lower(trim(raw.substr(0, pos)));
        std::string val = trim(raw.substr(pos+1));

        // remove aspas se houver
        if (!val.empty() && (val.front()=='"' || val.front()=='\'')) {
            char q = val.front();
            if (val.back()==q && val.size()>=2) val = val.substr(1, val.size()-2);
        }

        if (section.empty()) {
            if (key == "default_lang" && !val.empty()) {
                cfg.default_lang = val;
            }
        } else if (section == "aliases") {
            if (!key.empty() && !val.empty()) {
                cfg.aliases[key] = val;
            }
        }
    }
    return cfg;
}

// -------------------- embedded templates --------------------
static const std::unordered_map<std::string, std::string> EMBEDDED = {
    {"c",
R"(#include <stdio.h>
int main(void) {
    puts("Hello, World!");
    return 0;
}
)"},
    {"cpp",
R"(#include <iostream>
int main() {
    std::cout << "Hello, World!\n";
}
)"},
    {"rust",
R"(fn main() {
    println!("Hello, World!");
}
)"},
    {"python",
R"(print("Hello, World!")
)"},
};

// -------------------- template discovery --------------------
static std::vector<std::string> list_languages() {
    std::vector<std::string> langs;
    // embedded
    for (auto& kv : EMBEDDED) langs.push_back(kv.first);

    // user templates
    fs::path tdir = user_data_dir() / "templates";
    if (fs::exists(tdir) && fs::is_directory(tdir)) {
        for (auto& e : fs::directory_iterator(tdir)) {
            if (!e.is_regular_file()) continue;
            auto p = e.path();
            auto ext = to_lower(p.extension().string());
            if (ext == ".tmpl" || ext == ".txt") {
                langs.push_back(p.stem().string());
            }
        }
    }
    std::sort(langs.begin(), langs.end());
    langs.erase(std::unique(langs.begin(), langs.end()), langs.end());
    return langs;
}

static bool load_user_template(const std::string& lang, std::string& out) {
    fs::path base = user_data_dir() / "templates";
    std::vector<fs::path> candidates = {
        base / (lang + ".tmpl"),
        base / (lang + ".txt")
    };
    for (auto& p : candidates) {
        if (fs::exists(p) && fs::is_regular_file(p)) {
            return read_file(p, out);
        }
    }
    return false;
}

// -------------------- arg parsing --------------------
struct Args {
    bool show_list = false;
    std::string lang;      // --lang / -l
    std::string out_path;  // --out
    std::string alias_hit; // resolved via alias flags (-c, --cpp, --zig, etc.)
};

static void print_help(const char* prog, const Config& cfg) {
    std::cout
        << "Usage: " << prog << " [options]\n\n"
        << "Options:\n"
        << "  -l, --lang <name>   Linguagem (ex.: c, cpp, rust, python)\n"
        << "      --list          Lista linguagens disponíveis\n"
        << "      --out <file>    Salva a saída em arquivo\n"
        << "  -c                  Atalho para C (alias embutido)\n"
        << "      --cpp           Atalho para C++ (alias embutido)\n"
        << "\nAliases do config.ini ([aliases]):\n";
    if (cfg.aliases.empty()) {
        std::cout << "  (nenhum alias definido)\n";
    } else {
        for (auto& kv : cfg.aliases) {
            std::cout << "  --" << kv.first;
            if (kv.first.size() == 1) std::cout << "  (ou -"<< kv.first << ")";
            std::cout << "  -> " << kv.second << "\n";
        }
    }
    std::cout << "\nConfig e templates:\n"
#ifdef _WIN32
              << "  Config:    %APPDATA%\\hw\\config.ini\n"
              << "  Templates: %APPDATA%\\hw\\templates\\<lang>.tmpl\n";
#elif __APPLE__
              << "  Config:    ~/Library/Application Support/hw/config.ini\n"
              << "  Templates: ~/Library/Application Support/hw/templates/<lang>.tmpl\n";
#else
              << "  Config:    ~/.config/hw/config.ini\n"
              << "  Templates: ~/.local/share/hw/templates/<lang>.tmpl\n";
#endif
}

static Args parse_args(int argc, char** argv, const Config& cfg) {
    Args a;
    // para resolver aliases, percorremos argv manualmente
    for (int i = 1; i < argc; ++i) {
        std::string tok = argv[i];

        if (tok == "-h" || tok == "--help") {
            print_help(argv[0], cfg);
            std::exit(0);
        } else if (tok == "--list") {
            a.show_list = true;
        } else if (tok == "-l" || tok == "--lang") {
            if (i+1 >= argc) { std::cerr << "[hw] Faltou argumento para " << tok << "\n"; std::exit(1); }
            a.lang = argv[++i];
        } else if (tok == "--out") {
            if (i+1 >= argc) { std::cerr << "[hw] Faltou argumento para --out\n"; std::exit(1); }
            a.out_path = argv[++i];
        }
        // aliases embutidos:
        else if (tok == "-c") {
            a.alias_hit = "c";
        } else if (tok == "--cpp") {
            a.alias_hit = "cpp";
        }
        // aliases definidos no config:
        else if (starts_with(tok, "--")) {
            std::string name = tok.substr(2); // após --
            auto it = cfg.aliases.find(name);
            if (it != cfg.aliases.end()) a.alias_hit = it->second;
        } else if (starts_with(tok, "-") && tok.size() == 2) {
            // flag curta: -z
            std::string name(1, tok[1]);
            auto it = cfg.aliases.find(name);
            if (it != cfg.aliases.end()) a.alias_hit = it->second;
        } else {
            // ignorar argumentos posicionais
        }
    }
    return a;
}

// -------------------- main --------------------
int main(int argc, char** argv) {
    Config cfg = load_config();
    Args args = parse_args(argc, argv, cfg);

    if (args.show_list) {
        auto langs = list_languages();
        std::cout << "Linguagens disponíveis:\n";
        for (auto& l : langs) std::cout << " - " << l << "\n";
        return 0;
    }

    // Resolução de linguagem: --lang > alias > default_lang
    std::string lang;
    if (!args.lang.empty()) lang = args.lang;
    else if (!args.alias_hit.empty()) lang = args.alias_hit;
    else lang = cfg.default_lang.empty() ? "c" : cfg.default_lang;

    // tenta template do usuário
    std::string text;
    if (!load_user_template(lang, text)) {
        auto it = EMBEDDED.find(lang);
        if (it == EMBEDDED.end()) {
            std::cerr << "[hw] ERRO: linguagem '" << lang
                      << "' não encontrada. Use --list para ver opções.\n";
            return 1;
        }
        text = it->second;
    }

    if (!args.out_path.empty()) {
        if (!write_file(args.out_path, text)) {
            std::cerr << "[hw] ERRO: não consegui escrever em '" << args.out_path << "'\n";
            return 1;
        }
        std::cout << "[hw] Arquivo gerado: " << args.out_path << "\n";
    } else {
        std::cout << text;
    }

    return 0;
}

