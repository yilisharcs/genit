# Tasks

## bootstrap

```nu
if not ("nob" | path exists) {
    print "[INFO] --- BOOTSTRAPPING ---"
    cc -o nob nob.c
}
```

## build [profile]

```nu
mask bootstrap

if ($env.profile? | is-empty) {
    ./nob build
} else {
    ./nob build $env.profile
}
```

## run [profile]

```nu
mask bootstrap

if ($env.profile? | is-empty) {
    ./nob run
} else {
    ./nob run $env.profile
}
```

## clean

```nu
mask bootstrap

./nob clean
```
