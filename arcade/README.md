## 🕹️ Arcade Skin

<img src="reachy-mini-skin-arcade.png" alt="Reachy Mini arcade" width="250" align="left" style="margin-right: 20px;">

A retro arcade cabinet skin for Reachy Mini, featuring a 3D-printed arcade structure and an added screen.

The screen is connected directly to a PC, bringing the arcade effect to life. The antennas were also 3D printed to match the overall design.

<br clear="left">

---

### Steps to make it :

<details>
<summary><span style="font-size:1.2em"><b>BOM</b></span></summary>

#### 3D Printing
- PLA filament

#### Electronics
> *See [Electronics section](#electronics) below — component list to be added.*

#### Assembly
- Hot glue gun
- 4x screws for the screen (type and size TBD)

</details>

---

<details>
<summary><span style="font-size:1.2em"><b>3D Printing</b></span></summary>

STL and STEP files are available in the [`3d-models/`](3d-models/) folder.

| Part | File | Quantity | Supports |
|------|------|----------|----------|
| Arcade body | `reachy_mini_arcade.stl` / `reachy_mini_arcade.step` | 1x | Yes |
| Antenna | `antenna.stl` | 2x | Yes |

**Print settings:** Standard PLA settings. Supports are needed for both parts.

</details>

---

<details>
<summary><span style="font-size:1.2em"><b>Electronics</b></span></summary>

> **TODO** — This section will be completed with:
>
> - Component list (screen, connectors, wiring)
> - Wiring diagram
> - PlatformIO firmware code and flashing instructions
> - Screen configuration and PC connection setup
>
> *Stay tuned!*

</details>

---

<details>
<summary><span style="font-size:1.2em"><b>Assembly</b></span></summary>

1. **Prepare the parts**
   - Make sure all printed parts are ready
   - Remove supports and clean up the surfaces

2. **Replace the antennas**
   - Unscrew the stock antenna screws on Reachy Mini
   - Replace them with the 3D-printed antennas and screw them back in

3. **Mount the screen**
   - Screw the screen onto the arcade body (4x screws, type and size TBD)
   - Optionally, glue the screen cable in place to prevent it from moving around

4. **Attach the body skin**
   - Place the arcade body shell onto Reachy Mini
   - For now there is no snap or screw attachment — simply use a hot glue gun to secure the shell to the robot

</details>
