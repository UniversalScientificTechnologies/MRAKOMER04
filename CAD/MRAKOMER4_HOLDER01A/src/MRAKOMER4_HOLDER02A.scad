$fn=50;

senzor_prumer=8.2;
senzor_prumer_s_rantlem=9;

vyska_drzaku=45;

pruchodka_prumer=13.6;







MRAKOMER4_HOLDER02A();

module MRAKOMER4_HOLDER02A()
{
difference() {
    
union() { 
    
    translate([0,0,vyska_drzaku/2]) 
 cylinder(h=vyska_drzaku, r=pruchodka_prumer/2, center=true);

}
   translate([0,0,vyska_drzaku/2]) 
cylinder(h=vyska_drzaku+0.1, r=senzor_prumer/2, center=true);
  translate([0,0,vyska_drzaku-1/2-2]) 
cylinder(h=1, r=senzor_prumer_s_rantlem/2, center=true);

translate([0,0,vyska_drzaku/2]) 
  cube([pruchodka_prumer+2,0.5,vyska_drzaku+2],center=true); 
    }
   }
  
   
  