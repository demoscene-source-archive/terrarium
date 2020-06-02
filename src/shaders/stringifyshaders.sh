sed -r 's/\/\/.*//g; s/\s+/ /g; s/^\s//g; s/\\/\\\\/g; s/"/\\"/g; s/^/"/g; s/$/"/g; s/^"#version\s+430"$/"#version 430\\n"/g;' fragment.glsl > fragment.glsl.h
