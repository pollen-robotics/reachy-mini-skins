## 🎃 Halloween Pumpkin skin aka "Reachy-trouille"

<img src="reachy-mini-skin-halloween.png" alt="Reachy Mini pumpkin for Halloween" width="250" align="left">

A spooky jack-o’-lantern skin created for Halloween events, designed to give Reachy Mini a festive look. 

The body features an additional 3D-printed shell painted in orange, while the antennas were also 3D printed to form small bat wings. On the skin we made, we directly painted the head parts. But a thin 3D-printed cover can be added over the robot’s head as well, so there is no need to paint directly on the injected plastic shell.

<br clear="left">

### Steps to make it

1. **3D print the provided files**: Pumpkin, bat wings, top of the pumpkin

   You can print the parts directly in orange filament for the pumpkin, green filament for the top of the pumpkin, black filament for the bat wings, if you want to reduce the amount of painting.
   For reference, we printed ours in white.
   
3. **Paint the parts**

   If you did not print them in colors, you can paint them with spray paint. For reference, we used:

   - Pumkin and robot's face: Orange Cuivre RAL 2001 mat ([ref](https://www.bricorama.fr/p/peinture-aerosol-orange-cuivre-mat-400ml/3505390094594))
   - Top of the pumpkin: Pine Green RAL 6028 Satin ([ref](https://www.bricorama.fr/p/peinture-aerosol-vert-pin-satin-400ml/3505390094488))
   - Bat wings: Black Mat ([ref](https://www.leroymerlin.fr/produits/bombe-de-peinture-relook-tout-maison-deco-noir-mat-400-ml-70198814.html))
   - The result can be improved by painting in black or putting a black sticker/thin shell on the body surface, under the pumpkin skin, to hide the robot's original white face and make the pumpkin face stand out more.

5. **Insert pins**  
   - Insert pins in the holes of the body parts to allow them to snap together securely. You will need at least 4 pins 3mm x 16mm. 

6. **Assemble everything**  
   - Once all parts are printed, painted, and fitted with pins, assemble the skin onto Reachy Mini.
   - If you printed a cover for the head, you can stick it with thin double-sided tape.
   - The body parts should snap together with the pins
   - There is a channel inside the body shell to pass a string of light LEDs if you want to add some spooky lighting effects to your pumpkin skin. You can use a small battery pack to power the LEDs and hide it inside the body shell as well. We glued it to the inside of the pumpkin shell with glue gun, under the eyes and the mouth, to create a glowing effect. Make sure to leave the switch accessible so you can turn the lights on and off easily.

7. **Change the antennas motors configuration** (safety)  
   - If you want to make the bat wings move safely, you can [change the configuration of the antennas motors in the configuration file](https://github.com/pollen-robotics/reachy_mini/blob/1c347437881846b4ea3f293838baf11323c3e536/src/reachy_mini/assets/config/hardware_config.yaml#L144-L145), especially the limits of the motors position, to avoid any collision between the wings and the body.
