# main.cpp: How It Works

Every object is a **mesh**: a list of vertices and a list of faces. Each primitive builds its mesh once, `computeNormals` adds normals, and `drawMesh` sends it to OpenGL. Composite objects are copies of primitives placed with `glTranslatef`, `glRotatef` and `glScalef`.

## Data

**`Mesh`**
- `vertexAdjacency[v]`: the position of vertex `v`, and the faces that use it.
- `faceAdjacency[f]`: the vertex indices of face `f` (listed counter-clockwise when seen from outside), and the faces that share an edge with it.
- `faceNormals[f]`, `vertexNormals[v]`: filled in by `computeNormals`.

**Vector helpers:** `add`, `subtract`, `cross`, `dot`, `negate` and `normalize`, all on 3-component vectors.

$$
\mathbf a \times \mathbf b = (a_y b_z - a_z b_y,\; a_z b_x - a_x b_z,\; a_x b_y - a_y b_x)
\qquad
\mathbf a \cdot \mathbf b = a_x b_x + a_y b_y + a_z b_z
\qquad
\hat{\mathbf a} = \frac{\mathbf a}{\lVert \mathbf a \rVert}
$$

## Primitives

Each `buildX()` creates the mesh. Each `drawX(r, colour)` builds its mesh once (`static`), scales it by `r`, and calls `drawMesh`.

**`buildSphere`**: a unit sphere with $n = 20$.

$$
(x, y, z) = (\sin\theta \sin\phi,\; \cos\theta \sin\phi,\; \cos\phi),
\qquad \theta = \tfrac{i\pi}{n},\; i < 2n,
\qquad \phi = \tfrac{j\pi}{n},\; 0 \le j \le n
$$

- Each pole is a single vertex.
- The faces touching a pole lose their repeated corner and become triangles; all other faces are quads.

**`buildMobius`**: a strip with a half twist. $u$ goes around the loop and $v$ goes across the strip.

$$
(x, y, z) = \big((1 + v\cos\tfrac u2)\cos u,\; (1 + v\cos\tfrac u2)\sin u,\; v\sin\tfrac u2\big),
\qquad u \in [0, 2\pi),\; v \in [-0.4, 0.4]
$$

- At $u = 2\pi$ the strip meets its start with $v$ flipped. So the last column of faces joins row $j$ to row $m - j$ at $u = 0$.
- `drawMobius` turns on two-sided lighting, because both sides of the strip are visible.

**`buildChainLink`**: a round tube (radius $t = 0.2$) swept along a rounded-rectangle path, like a running track.
- The path is two straight sides at $x = \pm b$ and two half-circle ends of radius $b$ centred at $(0, \pm a)$.
- Each path point stores its centre $\mathbf c$ and the direction pointing out of the rectangle, $\mathbf o$.
- A ring of tube vertices is placed around each path point:

$$
\mathbf p = \mathbf c + t(\cos\psi\, \mathbf o + \sin\psi\, \hat{\mathbf z}), \qquad \psi = \tfrac{2\pi j}{m}
$$

- Indices wrap in both directions, so the surface is closed.

**`buildCylinder`**: unit radius, $y \in [-1.25, 1.25]$, with $\mathbf p = (\cos\theta, y, -\sin\theta)$.
- The side is made of quads.
- Each cap is a fan of triangles meeting at a centre vertex.
- The caps have their own copies of the rim vertices, so their normals are never averaged with the side's and the edges stay sharp.

## Normals and drawing

**`computeNormals`** (Step 1)

1. **Face normal** from the first three vertices $\mathbf a, \mathbf b, \mathbf c$ of the face:

$$
\mathbf n_f = \mathrm{normalize}\big((\mathbf b - \mathbf a) \times (\mathbf c - \mathbf a)\big)
$$

   Because the vertices go counter-clockwise, this normal points outward.

2. **Vertex normal**: the average of the normals of the $k$ faces around the vertex.

$$
\mathbf n_v = \mathrm{normalize}\Big(\tfrac1k \textstyle\sum_f \mathbf n_f\Big)
$$

   First, any $\mathbf n_f$ with $\mathbf n_f \cdot \mathbf n_{\text{first}} < 0$ is flipped. This matters only for the Möbius strip, which has no consistent "outside".

**`drawMesh(mesh, colour)`**
- Sets the colour as the material's ambient and diffuse colour.
- **Highlight on (H):** specular colour $(1, 1, 1)$ and shininess $64$. **Off:** specular colour $0$.
- **Flat (S off):** one normal per face, $\mathbf n_f$.
- **Smooth (S on):** one normal per vertex, $\mathbf n_v$, flipped if $\mathbf n_v \cdot \mathbf n_f < 0$.

**`setupLighting`**: sets up one point light at $(-100, 100, 100)$ with ambient $0.7$, diffuse $0.8$ and specular $1$.

For each light term, OpenGL multiplies the light's value by the material's. Summed, the colour is:

$$
I = L_a M_a + L_d M_d \max(\mathbf N \cdot \mathbf L, 0) + L_s M_s \max(\mathbf N \cdot \mathbf H, 0)^{\text{shininess}}
$$

Here $\mathbf N$ is the normal, $\mathbf L$ the direction to the light, and $\mathbf H$ the half-vector between $\mathbf L$ and the direction to the viewer.

## Composite objects

**`drawNunchuck`**: "Balance of Two"
- 7 chain links (scale $s = 0.25$), each $1.6s$ apart. $1.6$ is the link's inside length, so each link's end hooks inside the next one.
- Every second link is rotated $90^\circ$ about the chain's direction, so neighbouring links cross.
- Two stretched cylinders hang as handles, one under each end link.

**`drawAnchor`**: "Anchored in Gold"
- **Shank:** an upright cylinder.
- **Stock:** a cylinder rotated to run front-to-back, with a gold sphere on each end.
- **Arms:** 10 short cylinders placed along a circular arc of radius $R$ from $-160^\circ$ to $-20^\circ$.
  - Segment $k$ sits at $(R\cos\theta_k,\; y_c + R\sin\theta_k)$ and is rotated by $\theta_k$ to follow the arc.
  - Each is $2R\sin(\Delta\theta/2)$ long, the straight distance between its ends, scaled up by $1.1$ so neighbours overlap.
- **Crown:** a sphere covering the joint where the shank meets the arms.
- **Flukes:** flattened gold spheres at the arm tips.
- **Ring and chain:** a gold ring plus 2 silver chain links, each turned $90^\circ$ from the one below.

## Camera (Step 5)

**`updateProjection`**: `gluPerspective(fovy, 1, near, far)`.

**`setCamera(eye)`**: looks from `eye` towards the origin with $y$ as up. It also resets the mouse rotation and zoom.

**`resetCamera`** (`r`): camera at $(0, 0, 6)$, near $= 1$, far $= 80$, fovy $= 40^\circ$.

**`bestCamera`** (`R`): moves the camera to a view chosen for the current object.

**`display`**: calls `gluLookAt`, applies the mouse rotation and zoom, chooses flat or smooth shading, then draws the current object.

**`keyboard`**:

| Key | Action |
|---|---|
| `1`–`4` | Switch object, and change the window title to its name |
| `S` | Toggle flat / smooth shading |
| `H` | Toggle highlight |
| `n` / `N` | Near plane $\mp 0.25$ |
| `f` / `F` | Far plane $\mp 5$ |
| `o` / `O` | fovy $\mp 5^\circ$ |
| `r` / `R` | Reset camera / best view |

The limits keep $0.1 \le \text{near} < \text{far}$ and $5^\circ \le \text{fovy} \le 175^\circ$.

**`mouse` / `motion`**: left-drag rotates the object; right-drag zooms by scaling it.
