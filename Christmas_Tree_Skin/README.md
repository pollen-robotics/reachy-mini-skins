## 🎄Christmas Tree Skin

<img src="reachy-mini-skin-christmas.png" alt="Reachy Mini Christmas tree" width="250" align="left">

Give Reachy Mini a bold festive look with this shell customization. 

This skin combines 3D-printed parts, optional painting, and a magnetic star assembly for a polished final result.

<br clear="left">

### Steps to make it

1. **3D print the provided files**  
   You can print the parts directly in green filament if you want to reduce the amount of painting.  
   For reference, we printed ours in white.
   We printed the stars in golden filament.

3. **Paint the parts**  
   If you did not print them in green, you can paint them in green. For reference, we used:
   - **Color:** Pine Green
   - **Finish:** Satin  
   [Reference spray paint](https://www.bricorama.fr/p/peinture-aerosol-vert-pin-satin-400ml/3505390094488)

4. **Insert magnets and pins**  
   - Add magnets inside the stars so they align properly and form a star shape when the antennas touch. You will need 2 magnets [4mm x 3mm](https://www.amazon.fr/-/en/Wukong-Magnets-Neodymium-Printer-Project/dp/B0D31NCQNS/ref=sr_1_1?dib=eyJ2IjoiMSJ9.0l418wgre2ezKVAJ252rdTqy9hYeZRPdtP3OTXTm5qLMpjYvpeKHIYeouiyaqaDNEydn3xXfUYln0UN2_WIWpvPhAG_6kJSEiXWv9T8BwnNEpxSr22gbu2gAIWJYJ5GLRYQ4Y9oaaITfFdT6eHKfUbJF6kmDRdy_PiOVbEIcy_24jIAEhcPpo10DsIG_mGUtgCHgzwUejRKfBJ4IlCgByEKUn1jB_SYpXqdKDW2WPDI1VWHuL8m5hBJl4OCyD8UqliXH6TUqeZ8omU4CTwn1Ce-WOBbzJe6OzENg_PQKSJQ.MxQrpo7cTsCZjUJWOmqtEvBiOeYrJqd2rz3WVsz2Vns&dib_tag=se&keywords=4x3mm%2Bmagnet&qid=1774517546&sr=8-1&th=1). You can use super glue to secure them in place.
     - Make sure to orient the magnets correctly so that they attract each other when the antennas are close. 
   - Insert pins in the holes of the body parts to allow them to snap together securely. You will need at least 4 [pins 3mm x 16mm](https://www.amazon.fr/-/en/PATIKIL-Stainless-Cylindrical-Furniture-Positioning/dp/B0DHVNX6C5/ref=sr_1_3?crid=236XOH8C9KTXE&dib=eyJ2IjoiMSJ9.5QbO9V-ZiyqCYW4HSt7XFj0PMyGSZxoNFR6krojUaebGjuyvZbfKPrYNu9ISGgIByApxajzHGTS6gQU9latDMcaniepbV7bjWST988v-Xa-LN0LuUz44z1nv1blu7ksM1GVxGxKQ7ClkYF4vfifz4Jc25Hf4d__Kks8LtXZdgnatbaj95sC5q8Muumc6mH2Qail9L7ygE_S3JvxBI61vJLIGlqS4RyLXaIolorTsMNelthEaj9mLJ_0RGoNgCT9pXlFtJRCV4ngMD93anA_JjgtsLuV0Ub06_IvYOP6Fzrs.QLDjCGqnt6C1F0XYGhI4GEfoy8mFOTn99ZgfFp4K0Sc&dib_tag=se&keywords=goupille%2B3x16mm&qid=1774517633&sprefix=goupille%2B3x16mm%2Caps%2C227&sr=8-3&th=1). 

5. **Assemble everything**  
   - Once all parts are printed, painted, and fitted with magnets, assemble the skin onto Reachy Mini.
   - To assemble the head, you will need 3x self tapping screws 2.5mm x 6mm.
   - The body parts should snap together with the pins
   - The star must be pressed on the antennas, make sure to align the magnets correctly so they attract each other and form a star shape.
   - Some decorative elements can be added to the body parts if you wish, such as small ornaments or tinsel.

6. **Change the antennas motors configuration** (safety)  
   - If you want to make the antennas move safely, you can [change the configuration of the antennas motors in the configuration file](https://github.com/pollen-robotics/reachy_mini/blob/1c347437881846b4ea3f293838baf11323c3e536/src/reachy_mini/assets/config/hardware_config.yaml#L144-L145), especially the limits of the motors position, to avoid any collision between the stars/antennas and the body.