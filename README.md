# 🧠 hw — Hello World CLI

O **`hw`** (*Hello World*) é uma ferramenta de linha de comando (**CLI**) criada para imprimir exemplos básicos de programas “Hello, World!” em diferentes linguagens de programação.

A ideia é simples: você escolhe a linguagem e o comando mostra o código correspondente na tela — ou grava em um arquivo.
É um projeto pensado para crescer junto com o aprendizado de novas linguagens.

---

## ✨ Exemplo de uso

```bash
hw -c         # Exemplo básico em C
hw --cpp      # Exemplo básico em C++
hw --lang rust
hw --list     # Lista todas as linguagens disponíveis
hw --out main.c -c   # Salva o exemplo em um arquivo
````

Quando nenhum argumento é informado, o programa usa a linguagem padrão definida no arquivo de configuração.

---

## 🎯 Objetivos de design

O `hw` foi projetado para ser:

* **Fácil de usar** — suporte a flags curtas (`-c`, `--cpp`) e longas (`--lang rust`);
* **Fácil de estender** — novas linguagens podem ser adicionadas sem recompilar;
* **Portátil** — roda em Linux, macOS e Windows;
* **Versionável** — pode evoluir para um binário único distribuível.

---

## 🧩 Estrutura e funcionamento

### 1. Interface de linha de comando

* **Uso básico:**

  ```bash
  hw -c           # imprime exemplo em C
  hw --cpp        # imprime exemplo em C++
  hw --lang rust  # imprime exemplo em Rust
  hw              # usa linguagem padrão do config
  ```

* **Saídas:**

  * Padrão: imprime no terminal (`stdout`)
  * `--out`: grava o exemplo em arquivo (`hw --lang c --out main.c`)

* **Comandos úteis:**

  * `--list` → lista linguagens disponíveis
  * `--edit rust` → abre o template no editor padrão (planejado)

---

### 2. Onde ficam os templates

Os templates podem ser do **usuário** ou **embutidos** no programa.

1. **Templates do usuário**

   * Linux: `~/.local/share/hw/templates/`
   * macOS: `~/Library/Application Support/hw/templates/`
   * Windows: `%APPDATA%\hw\templates\`

2. **Templates embutidos (fallback)**

   * Usados quando o template não é encontrado no diretório do usuário.

Cada template corresponde a uma linguagem, com nome do arquivo igual ao identificador da linguagem (`c.tmpl`, `cpp.tmpl`, `rust.tmpl`, etc.).

**Exemplo (`c.tmpl`):**

```c
#include <stdio.h>
int main(void) {
    puts("Hello, World!");
    return 0;
}
```

**Exemplo (`cpp.tmpl`):**

```cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!\n";
}
```

---

### 3. Configuração do usuário

Arquivo TOML armazenado em:

* Linux: `~/.config/hw/config.toml`
* macOS: `~/Library/Application Support/hw/config.toml`
* Windows: `%APPDATA%\hw\config.toml`

**Exemplo:**

```toml
default_lang = "cpp"

[aliases]
c = "c"
cpp = "cpp"
cc = "cpp"
rs = "rust"
```

**Regras:**

* `default_lang`: define a linguagem padrão (quando nenhuma flag é passada)
* `[aliases]`: mapeia abreviações para linguagens

  * Ex.: `-c` → “c”; `--cpp` → “cpp”
* **Precedência:** `flag > alias > default_lang`

---

### 4. Extensibilidade sem recompilar

Adicionar uma nova linguagem é fácil:

1. Crie um arquivo `nova.tmpl` na pasta de templates do usuário;
2. (Opcional) adicione um alias no `config.toml`.

> 🔧 No futuro: o programa poderá suportar **plugins
>    executáveis**, tentando rodar `hw-<linguagem>` se não
>    encontrar o template.

---

### 5. Linguagens de implementação sugeridas

* **Python** → ideal para prototipar rápido (`argparse`, `pathlib`, `tomllib`);
  Pode ser empacotado em binário único com `pyinstaller`.
* **Go** → ótimo para binários portáteis e leves (segundo passo natural).
* **Rust** → excelente desempenho e segurança, mas com curva de aprendizado maior.

💡 **Sugestão prática:** comece em **Python** e, quando
o design estabilizar, porte para **Go**.

---

## 🧪 Protótipo funcional em Python (3.11+)

> Salve o script como `hw` (ou `hw.py`) e dê permissão de execução:
> `chmod +x hw`

Este protótipo:

* já vem com C, C++, Rust e Python embutidos;
* lê `config.toml` e templates personalizados, se existirem.

```python
# (código completo do protótipo Python)
```

---

### Exemplos de configuração e uso

**Configuração (`~/.config/hw/config.toml`):**

```toml
default_lang = "cpp"

[aliases]
c = "c"
cpp = "cpp"
rs = "rust"
py = "python"
```

**Template adicional (`~/.local/share/hw/templates/rust.tmpl`):**

```rust
fn main() {
    println!("Hello, World!");
}
```

**Comandos:**

```bash
hw          # usa a linguagem padrão (C++)
hw -c       # imprime em C
hw --cpp    # imprime em C++
hw --rs     # imprime em Rust
hw --lang python
hw --list
```

---

## 🚀 Próximos passos

* [ ] Autocompletar (`bash`, `zsh`, `fish`)
* [ ] Comando `--edit <lang>` para editar templates
* [ ] Plugins externos (`hw-<linguagem>`)
* [ ] Migração para Go (com Cobra/Viper) ou Rust (clap + confy)
* [ ] Testes unitários (validação de fallback e aliases)

---

## 📜 Licença

Este projeto é livre e aberto.
Você pode adaptá-lo, expandi-lo e usá-lo como base para
aprender CLI, TOML, templates e organização multiplataforma.

---

**Autor:** Ivan Carlos da Silva Lopes
**Objetivo:** aprender, crescer e documentar o estudo de
linguagens através de um único comando: `hw`.

```

