# 🖤 ASOC - Cybersecurity Course 

```
██╗   ██╗██╗██████╗ ██╗   ██╗███████╗    ██████╗  █████╗ ██╗   ██╗ █████╗ ██╗     
██║   ██║██║██╔══██╗██║   ██║██╔════╝    ██╔══██╗██╔══██╗╚██╗ ██╔╝██╔══██╗██║     
██║   ██║██║██████╔╝██║   ██║███████╗    ██████╔╝███████║ ╚████╔╝ ███████║██║     
╚██╗ ██╔╝██║██╔══██╗██║   ██║╚════██║    ██╔═══╝ ██╔══██║  ╚██╔╝  ██╔══██║██║     
 ╚████╔╝ ██║██║  ██║╚██████╔╝███████║    ██║     ██║  ██║   ██║   ██║  ██║███████╗
  ╚═══╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝    ╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚══════╝
                                                                                    
```

Welcome to the **ASOC** course repository — archived and operated by **Team Virus Payal**.
This repo collects weekly challenge branches, scripts, write-ups, and the blood, sweat and late-night terminal sessions that come with learning real-world cybersecurity.

---

## ⚙️ Repo philosophy (read this like a terminal)

* `main` — minimal: *only* docs and index.
* `week-{n}` — every week gets its **own branch** with exercises, scripts, and writeups.
* Commit messages should be concise and meaningful (think `feat: linux-privesc writeup`, `fix: exploit script args`).

---

## 🔪 Branch naming & workflow

Create a branch for each week:

```
git checkout -b week-01
# do the exercises
git add .
git commit -m "week-01: initial exercises and notes"
git push origin week-01
```

Branch naming convention:

```
week-1
week-2
week-3
...
```

If collaborating, use PRs to merge into `week-{n}` and keep `main` pristine.

---

## 🧩 What goes in each `week-{n}` branch

* `exercises/` — exercise files, scripts, payloads (clearly labeled).
* `writeups/` — markdown writeups detailing steps, commands used, lessons learned.
* `env/` — (optional) notes on VMs, docker-compose, or target setup (no sensitive secrets).
* `tools/` — small helpers and automation scripts (document usage in the README inside the branch).

---

## 🔐 Security & Safety rules (non-negotiable)

This is a learning repo. **Do not** include:

* Private keys, passwords, tokens, or any PII.
* Scripts intended for malicious real-world use outside of controlled lab environments.

Use responsible disclosure if you accidentally discover a real vulnerability. Always follow legal and ethical guidelines.

---

## 🛠 Tools & tech we use

* Python & Bash — automation and exploitation scripts.
* Git, GitHub — version control & collaboration.
* Docker / Vagrant — reproducible labs.

---

## ✍️ Commit & writeup style (keep it readable)

* Writeups should include: objective, environment, commands used, why it worked, defenses, mitigation.
* Use code blocks for commands and outputs. Strip secrets.
* Example header for a writeup:

```markdown
# Week 03 - Linux Privesc
**Target:** Ubuntu 20.04 (local VM)
**Objective:** Gain root via misconfigured sudoers
```

---

## 👥 Team — Virus Payal

* Lucas González Fiz
* [Add other members here]

Contact: add team alias or preferred contact method in branch `main` README (no personal secrets).

---

## 🧾 License & Use

This work is for **educational purposes only**. Use responsibly.
If you want to reuse code or writeups, attribute the team and never use the materials for malicious purposes.

---

## 🏴 Final note (a little terminal poetry)

```
> ./learn.sh --mode=aggressive
Loading…  
Exploit knowledge is a tool. Use it to defend, not to destroy.
— Virus Payal
```


