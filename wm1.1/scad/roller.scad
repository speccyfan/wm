$fn=100;

d=20;
h=12;

difference(){
union(){
	translate([0,0,1])cylinder(d1=0,d2=d,h=h,center=true);
	translate([0,0,-1])rotate([180,0,0])cylinder(d1=0,d2=d,h=h,center=true);

	translate([0,0,8.5])cylinder(d=d,h=3,center=true);
	translate([0,0,-8.5])cylinder(d=d,h=3,center=true);
}

cylinder(d=5.5,h=d,center=true);

translate([0,0,9])cylinder(d=14.1,h=8,center=true);
translate([0,0,-9])cylinder(d=14.1,h=8,center=true);

}

