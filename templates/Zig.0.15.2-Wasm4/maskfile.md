# Tasks

## build

```nu
zig build
```

## run

> Run the wasm cartridge

**OPTIONS**
* native
    * flags: -n --native
    * desc: Run a cartridge in the native desktop runtime

```nu
mask build

if not ($env.native? | is-empty) {
    w4 run-native zig-out/bin/cart.wasm
} else {
    w4 run zig-out/bin/cart.wasm
}
```

## open

```nu
job spawn { xdg-open http://localhost:4444 }

mask run
```

## bundle

**OPTIONS**
* native
    * flags: -n --native
    * desc: Bundle a cartridge as a native executable

```nu
if not ($env.native? | is-empty) {
    w4 bundle zig-out/bin/cart.wasm --title "GENIT_NAME" --windows GENIT_PKG-windows.exe --mac GENIT_PKG-mac --linux GENIT_PKG-linux
} else {
    w4 bundle zig-out/bin/cart.wasm --title "GENIT_NAME" --html GENIT_PKG.html
}
```

## clean

```nu
rm -rf .zig-cache/
rm -rf zig-out/
```
