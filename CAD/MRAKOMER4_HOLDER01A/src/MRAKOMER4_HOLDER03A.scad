$fn=50;


rozmer_x=65.8;
rozmer_y=42.8;
hrana_okraje=1.2;

vyska_soucastek=4;

M3_vyska_matky=3;
M3_sirka_matky=6.7;
M3_prumer_hlavy_sroubu=6;
M3_vyska_hlavy=5;
M3_prumer=3.2;

roztec_der=10.16;
sila_materialu=2;




MRAKOMER4_HOLDER03A();

module MRAKOMER4_HOLDER03A()
{
    
    
//obvod    
difference() {
    cube([rozmer_x+2,rozmer_y+2,vyska_soucastek+M3_vyska_matky+M3_vyska_hlavy+sila_materialu+1.4],center=false);
 
    
    //odstraneni vnitrni vyplne
   translate([hrana_okraje+1,hrana_okraje+1,0])  
cube([rozmer_x-2*hrana_okraje,rozmer_y-2*hrana_okraje,vyska_soucastek+M3_vyska_matky+M3_vyska_hlavy+sila_materialu+0.1],center=false);   
 
    //odstraneni rantlu
   translate([1,1,vyska_soucastek+M3_vyska_matky+M3_vyska_hlavy+sila_materialu])  
cube([rozmer_x,rozmer_y,vyska_soucastek+M3_vyska_matky+M3_vyska_hlavy+sila_materialu+0.1+1.4],center=false);     
    
    
    
    
    
    }
 
    
    //prostredni drzak
  difference(){
  union(){
    //spodni cast
     translate([rozmer_x/2-5,0,0])    
 cube([10,rozmer_y,sila_materialu],center=false);   
   
    
    //valec prostredni
   translate([rozmer_x/2,(rozmer_y+2)/2,0])   
   cylinder(h=vyska_soucastek+M3_vyska_matky+M3_vyska_hlavy+sila_materialu, r=(M3_sirka_matky+sila_materialu)/2, center=false);  
    
   //valec krajni 1
      translate([rozmer_x/2,5.08,0])   
   cylinder(h=M3_vyska_matky+sila_materialu, r=(M3_sirka_matky+sila_materialu)/2, center=false);   
      
        //valec krajni 2
      translate([rozmer_x/2,35.56,0])   
   cylinder(h=M3_vyska_matky+sila_materialu, r=(M3_sirka_matky+sila_materialu)/2, center=false); 
      
      
      
    }
    
    //otvor na matku
   translate([rozmer_x/2,(rozmer_y+2)/2,vyska_soucastek+M3_vyska_hlavy+sila_materialu])  
    cylinder (h = M3_vyska_matky+0.01, r= (M3_sirka_matky+0.2)/2, $fn=6); 
    
     //otvor na sroub
   translate([rozmer_x/2,(rozmer_y+2)/2,0])  
    cylinder (h = vyska_soucastek+M3_vyska_hlavy+sila_materialu+M3_vyska_matky+0.01, r= (M3_prumer+0.2)/2, $fn=50);  
    
    //otvor na hlavu šroubu
    translate([rozmer_x/2,(rozmer_y+2)/2,0]) 
     cylinder(h=M3_vyska_hlavy, r=M3_prumer_hlavy_sroubu/2, center=false);
    
     //valec krajni 1 - otvor na matku
 translate([rozmer_x/2,5.08,0]) 
    cylinder (h = M3_vyska_matky+0.01, r= (M3_sirka_matky+0.2)/2, $fn=6); 
   
    
         //valec krajni 2 - otvor na matku
  translate([rozmer_x/2,35.56,0])
    cylinder (h = M3_vyska_matky+0.01, r= (M3_sirka_matky+0.2)/2, $fn=6); 
    
   
    
    
    
     //valec krajni 1 - otvor na sroub
   translate([rozmer_x/2,5.08,0])   
    cylinder (h = vyska_soucastek+M3_vyska_hlavy+sila_materialu+M3_vyska_matky+0.01, r= (M3_prumer+0.2)/2, $fn=50);  
    
    
     //valec krajni 2 - otvor na sroub
   translate([rozmer_x/2,35.56,0])  
    cylinder (h = vyska_soucastek+M3_vyska_hlavy+sila_materialu+M3_vyska_matky+0.01, r= (M3_prumer+0.2)/2, $fn=50);  
    
    }
      
    
    
    }
 


  