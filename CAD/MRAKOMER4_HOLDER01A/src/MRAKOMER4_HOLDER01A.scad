$fn=50;
//otvory v krabičce
uchyt_roztec=85;
uchyt_prumer=4.5;


//otvordy pro PCB mrakomeru
PCB_prumer=3.4;
PCB_roztec=70;
PCB_vyska_soucastek=7;

vyska_materialu=3;
sire_materialu=15;


M3_vyska_matky=3;
M3_sirka_matky=6.7;
M3_prumer_hlavy_sroubu=6;
M3_vyska_hlavy=5;

//sloupek
sloupek_sirka=8;



MRAKOMER4_HOLDER01A();

module MRAKOMER4_HOLDER01A()
{
difference() {
    
union() { 

translate([0,0,vyska_materialu/2]) 
minkowski()
{
  cube([uchyt_roztec+uchyt_prumer,sire_materialu-8,vyska_materialu],center=true);
  cylinder(r=4,h=0.1);
}

translate([0,0,vyska_materialu/2+0.1]) 
  cube([PCB_roztec+PCB_prumer,sire_materialu,vyska_materialu+0.2],center=true);
  


translate([PCB_roztec/2,0,0]) 
SLOUPEK();  

translate([-PCB_roztec/2,0,0]) 
SLOUPEK(); 

}
   

//otvory pro uchycení PCB
translate([PCB_roztec/2,0,vyska_materialu/2]) 
cylinder(h=vyska_materialu+052, r=PCB_prumer/2, center=true);

translate([PCB_roztec/2,0,M3_vyska_hlavy/2])   
     cylinder(h=M3_vyska_hlavy, r=M3_prumer_hlavy_sroubu/2, center=true);

translate([-PCB_roztec/2,0,vyska_materialu/2]) 
cylinder(h=vyska_materialu+0.5, r=PCB_prumer/2, center=true);

translate([-PCB_roztec/2,0,M3_vyska_hlavy/2])   
     cylinder(h=M3_vyska_hlavy, r=M3_prumer_hlavy_sroubu/2, center=true);

//otvory pro uchycení ke krabičce
translate([uchyt_roztec/2,0,vyska_materialu/2]) 
cylinder(h=vyska_materialu+0.5, r=uchyt_prumer/2, center=true);

translate([-uchyt_roztec/2,0,vyska_materialu/2]) 
cylinder(h=vyska_materialu+0.5, r=uchyt_prumer/2, center=true);


    
    }
   }
  
   
  
    
 module SLOUPEK()  
   {
 
   difference() {
       translate([0,0,(vyska_materialu+PCB_vyska_soucastek)/2])   
    cube([sloupek_sirka,sire_materialu,vyska_materialu+PCB_vyska_soucastek],center=true);   
     
     
     translate([0,0,vyska_materialu+PCB_vyska_soucastek-M3_vyska_matky])    
    cylinder (h = M3_vyska_matky+0.01, r= (M3_sirka_matky+0.2)/2, $fn=6); 
     
         translate([0,0,(vyska_materialu+PCB_vyska_soucastek)/2])   
     cylinder(h=vyska_materialu+PCB_vyska_soucastek+0.2, r=PCB_prumer/2, center=true);
     
      translate([0,0,M3_vyska_hlavy/2])   
     cylinder(h=M3_vyska_hlavy, r=M3_prumer_hlavy_sroubu/2, center=true);
     
     
      
       
    }   
  }   
        