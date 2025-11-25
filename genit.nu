#!/usr/bin/env nu

def main [] {
        const script = (path self)
        const root = ([ ($script | path dirname) templates ] | path join)

        let template = (
                glob --depth=1 --no-file $"($root)/*"
                | each { path split | last }
                | sort
                | str join (char nl)
                | fzf --preview-window hidden;
        )
        print $"(ansi green_bold)=>(ansi reset) Selected: (ansi yellow_bold)($template)(ansi reset)"

        let name = (input $"(ansi green_bold)=>(ansi reset) Project Name: (ansi yellow_bold)")
        let repo = ($name | str replace --all --regex '\s+' '-')        # kebab-case eats dots, not ideal
        let rdir = ([".." $repo] | path join)
        let pkg  = ($repo | str downcase)

        let desc = (input $"(ansi green_bold)=>(ansi reset) Project Description: (ansi yellow_bold)")
        print --no-newline (ansi reset)

        if not ($rdir | path exists) {
                cp --recursive --no-clobber --preserve [ mode ] ([$root $template] | path join) $rdir
        } else {
                print $"(ansi red_bold) Error: (ansi yellow_bold)./($rdir) (ansi red_bold)exists.(ansi reset)"
                exit
        }

        cd $rdir

        # NOTE: We need to ignore the .gitignore rules to track the templates correctly
        mv .gitignore.dist .gitignore

        rg --no-follow --files-with-matches "GENIT_*"
        | lines
        | each {|e| $e
                | open --raw
                | lines
                | each {
                        if ($in | str contains "GENIT_") {
                                $in
                                | str replace --all "GENIT_NAME" $name
                                | str replace --all "GENIT_REPO" $repo
                                | str replace --all "GENIT_DESC" $desc
                                | str replace --all "GENIT_PKG"  $pkg
                                | str replace --all "GENIT_NVIM" ($pkg | str replace ".nvim" "")
                        } else {
                                return $in
                        }
                }
                | to text
                | collect
                | save --force $e
        }

        if $template == "Lua-nvim" or $template == "Lua" {
                mv GENIT_PKG-scm-1.rockspec $"($pkg)-scm-1.rockspec"
                rm doc/man; mkdir doc/man
        }

        if $template == "Lua-nvim" {
                mv lua/GENIT_NVIM.lua $"lua/($pkg | str replace ".nvim" "").lua"
                mv plugin/GENIT_NVIM.lua $"plugin/($pkg | str replace ".nvim" "").lua"
                make doc | ignore
        } else if $template == "Lua" {
                mv src/GENIT_PKG.lua $"src/($pkg).lua"
        }

        git init | ignore; git add .
        git commit -m "feat: initial commit" | ignore

        print $" (ansi green_bold)::(ansi reset) Created new project at (ansi green_bold)(pwd)(ansi reset)"
}
