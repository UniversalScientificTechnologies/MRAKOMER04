$fn=50;

senzor_prumer=8.5;
senzor_prumer_s_rantlem=9.3;

vyska_drzaku=73;

pruchodka_prumer=13.0;







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

translate([0,0,vyska_drzaku/2+20]) 
  cube([pruchodka_prumer+2,1,vyska_drzaku+2],center=true); 
    }
   }
  
   
  