#!/usr/bin/env nu

cc -M src/*.c
| lines
| str trim --char '\'
| str trim
| split row " "
| uniq
| sort
| where { not ($in | str ends-with ".o:") }
| do {
        let src_files = ($in | where { str ends-with ".c" })

        let p_headers = ($in
                | where { str ends-with ".h" }
                | where {
                        # System headers
                        ($in | str starts-with "/usr/") or not (
                                # Local headers without matching .c files (`foo.h`, no `foo.c`?)
                                ($in | path parse | upsert extension "c" | path join) | path exists)
                }
        )

        # 1st pass: no prototypes, yes signatures
        ctags --sort=no --kinds-C=-p --fields=+S -f tags ...$src_files

        # 2nd pass: yes prototypes, yes signatures for system and orphan headers
        ctags --sort=no --kinds-C=+p --fields=+S --append -f tags ...$p_headers
}

open tags
| lines
| uniq # Must deduplicate again
| do {
        let header = ($in | where { str starts-with "!" })

        # Sort here so .c files have priority over .h files
        let tags = ($in | where { not ($in | str starts-with "!")}
                | split column (char tab)
                | rename  --column { column0: tag }
                | sort-by tag
                | each {|row|
                        $row
                        | values
                        | str join (char tab)
                }
        )

        $header
        | append $tags
        | collect
        | save -f tags
}
