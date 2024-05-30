  int nb_program = 16; // nombre de programm
  int up_time = 1000; //init up_time
  int down_time = 1000; //init down_time
  int deltaSELECTEDPIX = 1;

  byte color_program_down[16][3] = {
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 0, 10},
    {10, 0, 0},
    {0, 10, 0},
    {0, 0, 10},
    {10, 0, 0}
  };
  byte color_program_up[16][3] = {
    {0, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0},
    {255, 0, 0},
    {100, 255, 0},
    {0, 0, 255},
    {255, 0, 0},
    {0, 255, 0}
  };
  int speed_program [16] = {0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 200, 200, 200, 1000, 1000};
  
void send_program(){



  if (programTIME_COUNT > programTIME_DELAY){ // test du temps de la boucle program
    
    programTIME_COUNT = 0; //reINIT de la boucle program
    if (program > nb_program){
      program_change(1); // retour au program 1
    } // reinit program = 1
    
    else if (program == -1){// PROGRAMpers 
      set_color_1pix(programSELECTEDPIX, down_time, color_program_down[0][0], color_program_down[0][1],color_program_down[0][2]); // pixel selectionné
      programSELECTEDPIX += deltaSELECTEDPIX;
      if (programSELECTEDPIX >= NUMPIXELS){
        programSELECTEDPIX = programSELECTEDPIX - NUMPIXELS;
      }
      set_color_1pix(programSELECTEDPIX, up_time, color_program_up[0][0], color_program_up[0][1], color_program_up[0][2]); // pixel suivant
    }
    else if (program == -2){// PROGRAMpers RDM
      set_color_1pix(programSELECTEDPIX, down_time, color_program_down[0][0], color_program_down[0][1],color_program_down[0][2]); // pixel selectionné
      programSELECTEDPIX = random(NUMPIXELS);
      set_color_1pix(programSELECTEDPIX, up_time, color_program_up[0][0], color_program_up[0][1], color_program_up[0][2]); // pixel suivant
    }
    else if ((program >= 1) && (program <= 5)){// 
      up_time = speed_program [program];
      down_time = up_time*5;
      programTIME_DELAY = up_time;
      set_color_1pix(programSELECTEDPIX, down_time, color_program_down[program][0], color_program_down[program][1],color_program_down[program][2]); // pixel selectionné
      programSELECTEDPIX += 1;
      if (programSELECTEDPIX >= NUMPIXELS){programSELECTEDPIX = programSELECTEDPIX - NUMPIXELS;      }
      set_color_1pix(programSELECTEDPIX, up_time, random(color_program_up[program][0]), random(color_program_up[program][1]), random(color_program_up[program][2])); // pixel suivant
    } // end  programme 1
    
    else if ((program >= 6) && (program<= 15)){// 
      up_time = speed_program [program];
      down_time = up_time*4;
      programTIME_DELAY = up_time;
      set_color_1pix(programSELECTEDPIX, down_time, color_program_down[program][0], color_program_down[program][1],color_program_down[program][2]); // pixel selectionné
      programSELECTEDPIX = random(NUMPIXELS);
      set_color_1pix(programSELECTEDPIX, up_time, random(color_program_up[program][0]), random(color_program_up[program][1]), random(color_program_up[program][2])); // pixel suivant
    } // end  programme 2

  } // END if (programTIME_COUNT > programTIME_DELAY){
  else{
  programcurrentMillis = millis(); // temps à t
  programTIME_COUNT = programTIME_COUNT - programpreviousMillis + programcurrentMillis; // 
  programpreviousMillis = programcurrentMillis;
  }
}
