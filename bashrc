###############################################
#  PowerShell‑Style Bash Configuration
#  Author: Scott + Copilot
###############################################

# ----- Colors -----
RESET="

\[\e[0m\]

"
FG_BLUE="

\[\e[38;5;39m\]

"
FG_CYAN="

\[\e[38;5;45m\]

"
FG_GRAY="

\[\e[38;5;250m\]

"
FG_YELLOW="

\[\e[38;5;221m\]

"

# ----- Windows‑style path formatter -----
function winpath() {
    echo "C:${PWD//\//\\}"
}

# ----- PowerShell‑style prompt -----
function prompt_ps() {
    local PATH_WIN=$(winpath)

    PS1="${FG_BLUE}PS ${FG_GRAY}${PATH_WIN}\n${FG_CYAN}> ${RESET}"
}

prompt_ps

# ----- PSReadLine‑style syntax highlighting -----
bind 'set colored-stats on'
bind 'set colored-completion-prefix on'
bind 'set completion-ignore-case on'
bind 'set show-all-if-ambiguous on'
bind 'set menu-complete-display-prefix on'

# Directory = PowerShell blue
# Executable = green
# Symlink = cyan
export LS_COLORS="di=38;5;39:fi=0:ln=38;5;45:pi=33:so=35:bd=33;1:cd=33;1:or=31;1:mi=31;1:ex=38;5;40"

# Strings = yellow (grep, etc.)
export GREP_COLOR='38;5;221'

# ----- Quality of life -----
shopt -s autocd
shopt -s cdspell
shopt -s dirspell
shopt -s checkwinsize

# ----- Aliases similar to PowerShell -----
alias ls='ls --color=auto'
alias dir='ls -l'
alias copy='cp'
alias move='mv'
alias del='rm -i'
alias cls='clear'

###############################################
# End of PowerShell‑Style Bash Configuration
###############################################
