T = 3;
motor_opening_d = 64;
motor_offset = 6;
corner_r = 8;
cube_w = motor_opening_d + 2*T - 2*corner_r;
w = cube_w + 2*corner_r;
electronics_w = 32;
cube_l = cube_w+motor_offset+electronics_w+T;
l = cube_l + 2*corner_r;
button_d = 12.5;


module motor() {
  d1 = 59;
  d2 = 53;
  out_d = 22;
  h2 = 10.5;
  h1 = 44;
  out_h = 4.5;
  cylinder(h1, d1/2, d1/2);
  translate([0,0,h1]) cylinder(h2, d2/2, d2/2);
  translate([0,0,h1+h2]) cylinder(out_h, out_d/2, out_d/2);
}

module motor_mounts() {
  for (i=[0:2]) translate([i*7-10,15,0]) 
  difference() {
    cube([6, 19, 6]);
    translate([2,0,3]) cube([2, 19, 3]);
  }
  difference() {
    cylinder(5, 22/2+T, 22/2+1);
    cylinder(5, 22/2, 22/2);
  }
}

module display() {
  translate([-28/2,-14/2-5,-3]) {
    difference() {
      cube([28,28,3]);
      for (i=[2, 28-2]) for (j=[2, 28-2]) translate([i,j,1]) cylinder(2,.1,1, $fn=20);
    }
    translate([1,5,0]) cube([26,14,6]);
  }
}

module lid_cutout() {
      translate([corner_r, corner_r, 0]) minkowski() {
        cube([cube_w,cube_l,100]);
        cylinder(r=corner_r-2,h=T);
      }
}

module shell() {
  motor_top_inset = 16;
  motor_opening_h = 45+4 + motor_top_inset;
  h = motor_opening_h + T;
  filter_l = 54;
  filter_w = 35;
  filter_t = 6;

  translate([0,0,-h]) difference() {
    union() {
      translate([corner_r, corner_r, 0]) minkowski() {
        cube([cube_w,cube_l,h]);
        cylinder(r=corner_r,h=T);
      }
    }
    translate([corner_r, 2*T+motor_opening_d+motor_offset+(corner_r-T), T]) minkowski() {
      cube([cube_w,cube_l-motor_opening_d-motor_offset-T,h-T-1]);
      cylinder(r=corner_r-T,h=1);
    }
    translate([motor_opening_d/2+T,motor_offset+motor_opening_d/2+T,T]) cylinder(h=100, r=motor_opening_d/2);
    
    translate([0,0,h]) lid_cutout();
        
    // motor notches
    for(i=[0,120,240]) translate([(motor_opening_d+2*T)/2,motor_offset+(motor_opening_d+2*T)/2,h]) 
    rotate([0,0,i-30]) translate([(motor_opening_d)/2,0,0]) cube([2,6,(15+4*2+motor_top_inset)*2], center=true);
    
    // filter
    hole_r = 2.5;
    translate([(motor_opening_d+2*T-filter_l)/2,2,0]) cube([filter_l, filter_t, filter_w]);
    //translate([(motor_opening_d+2*T-hole_r*2)/2,0,0]) cube([hole_r*2, filter_t, 7]);
    for (i=[-3:3]) for (j=[0,1,2])
    translate([i*7.5+(motor_opening_d+2*T)/2,0,7+j*12]) rotate([-90,0,0]) cylinder(20, r=hole_r, $fn=20);
    for (i=[-3:2]) for (j=[0,1])
    translate([(i+.5)*7.5+(motor_opening_d+2*T)/2,0,13+j*12]) rotate([-90,0,0]) cylinder(20, r=hole_r, $fn=20);
    
    // wires
    for (i=[1:3]) translate([i*10+T+3,motor_opening_d-15,T+3]) rotate([-90,0,0]) cylinder(40, r=4);
      
    translate([(motor_opening_d+2*T)/2,cube_l+corner_r*2-1,(h+T)/2]) mirror([0,1,0]) rotate([90,90,0]) display();

    // button
    translate([(motor_opening_d+2*T)/2,cube_l+corner_r*2-18,(h+T)/2]) rotate([0,90,0]) cylinder(h=100, r=button_d/2);

    // screw holes
    for (i=[0,1]) for (j=[0,1]) translate([corner_r +i*cube_w,corner_r+j*(cube_w+3*T),h-17]) cylinder(20,r=1, $fn=10);
        
    // power adapter
    translate([T+15/2,l-electronics_w/2-T,-T]) cylinder(T*2, r=11.5/2);
    
  }
}

module lid() {
  air_hole_h = 3;
  intersection() {
      lid_cutout();
      difference() {
        union() {
          translate([corner_r, corner_r, 0]) minkowski() {
            cube([cube_w,cube_l,T-1]);
            cylinder(r=corner_r,h=1);
          }
          translate([corner_r+cube_w/2,motor_offset+corner_r+cube_w/2,0]) {
            cylinder(air_hole_h+T, 22/2+2*T, 22/2+1+T);
            cylinder(air_hole_h+T+20, 25/2, 23/2);
            pressure_tube_l = motor_opening_d/2+T+electronics_w/2;
            translate([0,0,air_hole_h]) rotate([-90,0,0]) cylinder(pressure_tube_l, r=3, $fn=20);
          }
        }
        // screw holes
        for (i=[0,1]) for (j=[0,1]) translate([corner_r +i*cube_w,corner_r+j*(cube_w+3*T),0]) cylinder(20,r=1, $fn=10);

        translate([corner_r+cube_w/2,motor_offset+corner_r+cube_w/2,0]) {
          cylinder(100, r=19/2);
          translate([0,0,air_hole_h]) rotate([-90,0,0]) cylinder(cube_w-6, r=1.6, $fn=20);
          translate([0,cube_w-7,0]) cylinder(air_hole_h, r=1.6, $fn=20);
        }

      }
  }
}

//motor();
motor_mounts();
//shell();
//translate([0,0,2*T]) lid();
//display();
