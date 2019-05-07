$fn=50;

senzor_prumer=7;
senzor_prumer_s_rantlem=7;

vyska_drzaku=10;

pruchodka_prumer=9.5;
pruchodka_rantl_prumer=12;






MRAKOMER4_HOLDER02A();

   
   
module MRAKOMER4_HOLDER02A()
{
difference() {
    
union() { 
    
    translate([0,0,vyska_drzaku/2]) 
 cylinder(h=vyska_drzaku, r=pruchodka_prumer/2, center=true);
    
    translate([0,0,2/2]) 
 cylinder(h=2, r=pruchodka_rantl_prumer/2, center=true);
    

}
   translate([0,0,vyska_drzaku/2]) 
cylinder(h=vyska_drzaku+0.1, r=senzor_prumer/2, center=true);
  translate([0,0,vyska_drzaku-1/2-2]) 
cylinder(h=1, r=senzor_prumer_s_rantlem/2, center=true);

translate([0,0,vyska_drzaku/2+20]) 
  cube([pruchodka_prumer+2,1,vyska_drzaku+2],center=true); 
    }
   }
  
   
  