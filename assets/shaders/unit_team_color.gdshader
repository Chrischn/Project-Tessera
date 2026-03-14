// =============================================================================
// unit_team_color.gdshader
// Spatial shader for Civ IV units that carry a TeamColor.bmp in NIF slot 0.
//
// Blend formula: ALBEDO = mix(team_color, base_texture, base.a)
//   base.a == 0.0 → team-colorable area (cloth/fabric) → show team_color
//   base.a == 1.0 → base texture area (armor/horse)    → show base.rgb
//
// The DECAL_0 texture (albedo_texture) uses DXT3 (BC2) compression which
// stores 4-bit explicit alpha per pixel.  Artists paint team-colorable areas
// as transparent (alpha=0) so the team color shows through as a background
// layer.  Non-team areas are opaque (alpha=255) and show the base texture.
//
// TeamColor.bmp (dark_map uniform) is a 2x2 solid-color marker with no
// per-pixel mask data; it is kept as a uniform so the C++ side can keep
// passing it without changes, but it is not used in the blend calculation.
// =============================================================================
shader_type spatial;
render_mode cull_back;

// DECAL_0 — actual unit diffuse texture, DXT3 alpha = team-color mask
uniform sampler2D albedo_texture  : source_color, filter_linear_mipmap;
// Kept for C++ compatibility; not used in blend (see header comment above)
uniform sampler2D dark_map        : hint_default_black, filter_linear_mipmap;
// Normal/bump map — only sampled when has_normal_map is true; avoids
// degenerate zero-normal from an unset sampler killing diffuse lighting.
uniform sampler2D normal_texture  : hint_roughness_normal, filter_linear_mipmap;
uniform bool has_normal_map = false;

uniform vec4  team_color               : source_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float roughness                : hint_range(0.0, 1.0) = 0.5;
uniform float specular_intens          : hint_range(0.0, 1.0) = 0.5;

void fragment() {
    vec4 base = texture(albedo_texture, UV);

    // base.a is the DXT3 per-pixel team-color mask:
    //   0.0 → cloth/fabric → show team_color
    //   1.0 → armor/horse  → show base.rgb
    ALBEDO    = mix(team_color.rgb, base.rgb, base.a);
    // No ALPHA write — mesh is always opaque.  Writing ALPHA (even 1.0)
    // would push the mesh into Godot's transparent pipeline, breaking depth.
    ROUGHNESS = roughness;
    SPECULAR  = specular_intens;

    if (has_normal_map) {
        NORMAL_MAP = texture(normal_texture, UV).rgb;
    }
}
