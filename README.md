<h1> CURRENTLY UNDER CONSTRUCTION AND NOT BATTLE TESTED WAIT FEW DAYS </h1>
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
- Few signals to use as a gui application. 

, 




### COSEC;
cosec is the popup gui window for crashed application it uses the signal given by cos and uses it for gui overview it heavily uses qt and also not that well structured. 

use 
#include <COSEC>
or #include <trigonometry/COSEC>
but remember, it uses qt for gui so then, you will need dev-qt6-core. 
