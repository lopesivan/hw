#!/usr/bin/env python
import argparse
import os
import sys
from pathlib import Path

# --- Util: paths por OS


def user_config_dir() -> Path:
    if sys.platform.startswith("linux"):
        return Path.home() / ".config" / "hw"
    elif sys.platform == "darwin":
        return Path.home() / "Library" / "Application Support" / "hw"
    else:
        return Path(os.environ.get("APPDATA", str(Path.home()))) / "hw"


def user_data_dir() -> Path:
    if sys.platform.startswith("linux"):
        return Path.home() / ".config" / "hw" / "templates"
    elif sys.platform == "darwin":
        return Path.home() / "Library" / "Application Support" / "hw"
    else:
        return Path(os.environ.get("APPDATA", str(Path.home()))) / "hw"

# --- Carregar config TOML (Python 3.11+ tem tomllib)


def load_config():
    cfg = {"default_lang": "c", "aliases": {}}
    cfg_path = user_config_dir() / "config.toml"
    if cfg_path.exists():
        try:
            import tomllib
            data = tomllib.loads(cfg_path.read_text(encoding="utf-8"))
            if isinstance(data, dict):
                cfg.update({k: v for k, v in data.items()
                           if k in ("default_lang", "aliases")})
                if "aliases" in cfg and cfg["aliases"] is None:
                    cfg["aliases"] = {}
        except Exception as e:
            print(f"[hw] Aviso: falha ao ler {cfg_path}: {e}", file=sys.stderr)
    return cfg


# --- Templates embutidos (fallback)
EMBEDDED = {
    "c": """#include <stdio.h>
int main(void) {
    puts("Hello, World!");
    return 0;
}
""",
    "cpp": """#include <iostream>
int main() {
    std::cout << "Hello, World!\\n";
}
""",
    "rust": """fn main() {
    println!("Hello, World!");
}
""",
    "python": """print("Hello, World!")
""",
}

# --- Carregar template de arquivo do usuário, se existir


def load_user_template(lang: str) -> str | None:
    # hw/templates/<lang>.tmpl
    base = user_data_dir() / "templates"
    candidates = [
        base / f"{lang}.tmpl",
        base / f"{lang}.txt",
    ]
    for p in candidates:
        if p.exists():
            return p.read_text(encoding="utf-8")
    return None


def resolve_lang(args, cfg):
    # 1) flag explícita --lang
    if args.lang:
        return args.lang
    # 2) flags curtas do tipo --cpp, -c, etc.: mapeadas por aliases
    # mapeio a presença de flags booleanas
    # exemplo: --cpp vira args.cpp = True
    alias_hit = None
    for k, v in cfg.get("aliases", {}).items():
        # se o alias for usado como --<k> (ex.: --cpp), argparse registra True:
        if getattr(args, k.replace("-", "_"), False):
            alias_hit = v
            break
    if alias_hit:
        return alias_hit
    # 3) default do config
    return cfg.get("default_lang", "c")


def available_languages(cfg):
    # união de embutidos + templates do usuário
    langs = set(EMBEDDED.keys())
    tdir = user_data_dir() / "templates"
    if tdir.exists():
        for p in tdir.iterdir():
            if p.is_file() and p.suffix in (".tmpl", ".txt"):
                langs.add(p.stem)
    # aliases também contam como “atalhos”, mas não como linguagem em si
    return sorted(langs)


def main():
    cfg = load_config()

    parser = argparse.ArgumentParser(
        prog="hw",
        description="Imprime um Hello World na linguagem escolhida."
    )
    parser.add_argument(
        "-l", "--lang", help="nome da linguagem (ex.: c, cpp, rust, python)")
    parser.add_argument("--list", action="store_true",
                        help="lista linguagens disponíveis")
    parser.add_argument("--out", help="salva a saída em arquivo")
    # aliases de exemplo: -c e --cpp (você pode aumentar via config.toml)
    # Dica: mantenha em sincronia com seu config.toml
    parser.add_argument("-c", dest="c", action="store_true",
                        help="atalho para C (alias)")
    parser.add_argument("--cpp", dest="cpp",
                        action="store_true", help="atalho para C++ (alias)")

    # também adiciona dinamicamente aliases definidos no config (para flags longas)
    for alias in cfg.get("aliases", {}).keys():
        if alias in ("c", "cpp"):  # já adicionados acima
            continue
        # cria flags longas: --rs, --zig, etc.
        parser.add_argument(f"--{alias}", dest=alias.replace("-", "_"), action="store_true",
                            help=f"atalho para linguagem mapeada em '{alias}' no config")

    args = parser.parse_args()

    if args.list:
        langs = available_languages(cfg)
        print("Linguagens disponíveis:")
        for l in langs:
            print(f" - {l}")
        return 0

    lang = resolve_lang(args, cfg)

    # tenta user template primeiro
    text = load_user_template(lang)
    if text is None:
        # cai para embutido
        text = EMBEDDED.get(lang)

    if text is None:
        print(
            f"[hw] ERRO: linguagem '{lang}' não encontrada. Use --list para ver opções.", file=sys.stderr)
        return 1

    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
        # mensagem simples para UX
        print(f"[hw] Arquivo gerado: {args.out}")
    else:
        print(text, end="")

    return 0


if __name__ == "__main__":
    sys.exit(main())
