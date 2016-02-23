$fn=100;

d=25;
h=20;

difference(){
union(){
	translate([0,0,0])cylinder(d1=8,d2=d,h=h,center=true);
	translate([0,0,11.5])cylinder(d=d,h=3,center=true);

}

cylinder(d=5.5,h=h+10,center=true);

}

