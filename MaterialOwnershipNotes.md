# Renderer Material Ownership Notes

## Current Issue

Material properties should have a clear owner. In the previous structure,
`diffuse` and `specular` were stored as `Ryudar` member variables, while
`shininess` was edited directly through `meshGroup.m_basicPixelConstantData.material`.

That meant:

| Property | Owner | Effect |
|---|---|---|
| Diffuse | `Ryudar` | Shared by sphere and character |
| Specular | `Ryudar` | Shared by sphere and character |
| Shininess | `BasicMeshGroup` material | Separate per object |

This is confusing because all three values describe the surface of a material,
but they were not owned by the same object.

## Usual Renderer Design

In most renderers, material properties belong to the object, mesh, submesh, or
material instance. They are not usually global renderer state.

Examples:

```text
Sphere Material
  diffuse
  specular
  shininess

Character Material
  diffuse
  specular
  shininess
```

This allows different objects to have different surface properties.

## Single Source Of Truth

The safest structure is to keep the actual render state in one place.

For this project, the natural owner is:

```cpp
meshGroup.m_basicPixelConstantData.material
```

The GUI should edit that material directly, and the constant buffer update
should upload that material to the GPU.

## Current Improvement

The GUI now edits all three material values directly:

```cpp
auto &material = meshGroup.m_basicPixelConstantData.material;

ImGui::SliderFloat3("Material Diffuse", &material.diffuse.x, 0.0f, 3.0f);
ImGui::SliderFloat3("Material Specular", &material.specular.x, 0.0f, 3.0f);
ImGui::SliderFloat("Material Shininess", &material.shininess, 0.01f, 20.0f);
```

This means the sphere and character can each keep their own material values.

## Color And Strength Split

Real material tools often separate color from intensity.

Example:

```text
Diffuse Color + Diffuse Strength
Specular Color + Specular Strength
Shininess or Roughness
```

If that is added later, be careful not to store only one editable material in
`Ryudar`, because that would make all objects share the same values again.

The editable material should also be owned per object or per material instance.

```text
Sphere
  EditableMaterial
  GPU Material

Character
  EditableMaterial
  GPU Material
```

## HLSL Compatibility

The current C++ `Material` struct matches the HLSL `Material` struct used in
the constant buffer.

If the GPU-facing `Material` struct changes, the HLSL struct must also change.

To keep HLSL unchanged, use two layers:

```text
EditableMaterial in C++
  diffuseColor
  diffuseStrength
  specularColor
  specularStrength

GPU Material sent to HLSL
  diffuse = diffuseColor * diffuseStrength
  specular = specularColor * specularStrength
```

This keeps the shader logic stable while improving the editor/UI model.

## Interview Answer

Material properties such as diffuse, specular, and shininess describe the
surface of an object, so I would normally let the material or mesh own them
instead of storing them as renderer-wide state.

The important design point is to keep a single source of truth. If UI state and
rendering state are separated unnecessarily, synchronization bugs can appear.
So the GUI should edit the actual material data, or if an editable material
layer exists, it should still be owned per material instance and converted to
GPU material data before upload.
