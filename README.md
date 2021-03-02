# Update
With recent releases of Mesa, you can now just use something like
`MESA_GL_VERSION_OVERRIDE=3.3COMPAT`.

# wine_mesa_discard_profiles
Discards OpenGL profile masks when creating contexts under Wine with Mesa.

Mesa currently lacks support for OpenGL compatibility profiles with Intel
integrated graphics.

Certain Windows programs fail to initialize when they try to obtain
compatibility profiles under Wine. Some of these programs are able to function
without visible side effects when this request is silently ignored.

This is a small `LD_PRELOAD` library that forces use of only the core profile by
wrapping `dlsym()` and some GLX functions as used by Wine.


## Usage
```sh
make
LD_PRELOAD="$PWD/wine_mesa_discard_profiles.so" wine [...]
```
