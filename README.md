<h1> CURRENTLY UNDER CONSTRUCTION AND NOT BATTLE TESTED WAIT FEW DAYS </h1>
<img width="1280" height="800" alt="test-os-for-trig" src="https://github.com/user-attachments/assets/0f83521a-bf36-4622-b548-38fc28ab3491" />
# libtrigonometry
is nothing but a fun project also my way to learn more about unix shared  Library


# `libtrigonometry`Contains for now, 
COS (**central output service**) 
COSEC ( **crash output servicer** and **easy calibrator** ) 


with  .so files and  fully configured cmake for it. 

also there is a   .deb folder in this repo make sure to know that that's for those who wants to install in debian and don't wanna configure anymore also that's the source code, 

---

as for development, 
type
#include <COS> or #include <trigonometry/COS> to include it as a header
benifits? 
- outputting the application output into a logger 
- when started finished with time stamps 
- A crash recognizer
- Few signals to use as a gui application. a fully featured restart, terminate flag that is free form any frameworks  [  Trig_term();  Trig_reset();  ] 

, 


### COSEC;
cosec is the popup gui window for crashed application it uses the signal given by cos and uses it for gui overview it heavily uses qt and also not that well structured. 
<img width="729" height="515" alt="image" src="https://github.com/user-attachments/assets/042bda71-a8e7-481e-a31d-74771b2480ed" />
<img width="766" height="519" alt="image" src="https://github.com/user-attachments/assets/2d54f818-f633-4888-82e0-c5d97fab7cbe" />
<img width="765" height="522" alt="image" src="https://github.com/user-attachments/assets/437b912f-4a0c-42a4-84a0-21db0e34340f" />

use 
#include <COSEC>
and in your mainwindow class add

REG_CRASH(); 
it means register for crash utilities operation ( 
but remember, it uses qt for gui so then, you will need dev-qt6-core. 





















