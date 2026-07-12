float fader_max_dB = 10.0;
float gain_deg_per_dB = 5.0; // Conversion factor

// ==========================================
// CHANNEL 1 PROCESSING
// ==========================================
// 1. Calculate Channel 1 physical limits
float L1 = Max_Fader_Length_mm_1; 
float p100_1 = L1;           
float p75_1  = L1 * 0.75;    
float p25_1  = L1 * 0.25;    
float p05_1  = L1 * 0.05;    

// 2. REVERSE MAPPING (Where is the fader right now?)
float Fader_Current_dB_1;
if (Fader_Current_mm_1 >= p75_1) {
    Fader_Current_dB_1 = ((Fader_Current_mm_1 - p75_1) / (p100_1 - p75_1)) * 10.0;
} else if (Fader_Current_mm_1 >= p25_1) {
    Fader_Current_dB_1 = ((Fader_Current_mm_1 - p25_1) / (p75_1 - p25_1)) * 30.0 - 30.0;
} else if (Fader_Current_mm_1 >= p05_1) {
    Fader_Current_dB_1 = ((Fader_Current_mm_1 - p05_1) / (p25_1 - p05_1)) * 30.0 - 60.0;
} else {
    Fader_Current_dB_1 = -60.0; 
}

// NEW: REVERSE MAPPING (Where is the gain knob right now?)
float Gain_Current_dB_1 = Gain_Current_deg_1 / gain_deg_per_dB;

// 3. Audio Math (Calculate True Total dB)

float delta_dB_1 = 20 * log10(V_avg_1 / V_ref_1);
float total_req_dB_1 = Fader_Current_dB_1 - delta_dB_1;

float target_fader_dB_1;
float target_gain_dB_1;

// 4. Spillover Logic (Drop Gain first, then Fader)
if (total_req_dB_1 > fader_max_dB) {
    target_fader_dB_1 = fader_max_dB;
    target_gain_dB_1 = (total_req_dB_1 - fader_max_dB)+ Gain_Current_dB_1;
} else {
    target_fader_dB_1 = total_req_dB_1;
    target_gain_dB_1 = Gain_Current_dB_1; 
}

// 5. FORWARD MAPPING (Convert new dB targets back to physical movement)
float target_mm_1;
if (target_fader_dB_1 >= 0.0) {
    target_mm_1 = p75_1 + ((target_fader_dB_1 - 0.0) / 10.0) * (p100_1 - p75_1);
} else if (target_fader_dB_1 >= -30.0) {
    target_mm_1 = p25_1 + ((target_fader_dB_1 - (-30.0)) / 30.0) * (p75_1 - p25_1);
} else if (target_fader_dB_1 >= -60.0) {
    target_mm_1 = p05_1 + ((target_fader_dB_1 - (-60.0)) / 30.0) * (p25_1 - p05_1);
} else {
    target_mm_1 = 0.0; 
}

Fader_Target_mm_1 = target_mm_1;
Gain_Target_deg_1 = target_gain_dB_1 * gain_deg_per_dB;


// ==========================================
// CHANNEL 2 PROCESSING
// ==========================================
// 1. Calculate Channel 2 physical limits
float L2 = Max_Fader_Length_mm_2; 
float p100_2 = L2;           
float p75_2  = L2 * 0.75;    
float p25_2  = L2 * 0.25;    
float p05_2  = L2 * 0.05;    

// 2. REVERSE MAPPING (Where is the fader right now?)
float Fader_Current_dB_2;
if (Fader_Current_mm_2 >= p75_2) {
    Fader_Current_dB_2 = ((Fader_Current_mm_2 - p75_2) / (p100_2 - p75_2)) * 10.0;
} else if (Fader_Current_mm_2 >= p25_2) {
    Fader_Current_dB_2 = ((Fader_Current_mm_2 - p25_2) / (p75_2 - p25_2)) * 30.0 - 30.0;
} else if (Fader_Current_mm_2 >= p05_2) {
    Fader_Current_dB_2 = ((Fader_Current_mm_2 - p05_2) / (p25_2 - p05_2)) * 30.0 - 60.0;
} else {
    Fader_Current_dB_2 = -60.0; 
}

// NEW: REVERSE MAPPING (Where is the gain knob right now?)
float Gain_Current_dB_2 = Gain_Current_deg_2 / gain_deg_per_dB;

// 3. Audio Math (Calculate True Total dB)
 
float delta_dB_2 = 20 * log10(V_avg_2 / V_ref_2);
float total_req_dB_2 = Fader_Current_dB_2 - delta_dB_2;

float target_fader_dB_2;
float target_gain_dB_2;

// 4. Spillover Logic (Drop Gain first, then Fader)
if (total_req_dB_2 > fader_max_dB) {
    target_fader_dB_2 = fader_max_dB;
    target_gain_dB_2 = (total_req_dB_2 - fader_max_dB) + Gain_Current_dB_2;
} else {
    target_fader_dB_2 = total_req_dB_2;
    target_gain_dB_2 = Gain_Current_dB_2; 
}

// 5. FORWARD MAPPING (Convert new dB targets back to physical movement)
float target_mm_2;
if (target_fader_dB_2 >= 0.0) {
    target_mm_2 = p75_2 + ((target_fader_dB_2 - 0.0) / 10.0) * (p100_2 - p75_2);
} else if (target_fader_dB_2 >= -30.0) {
    target_mm_2 = p25_2 + ((target_fader_dB_2 - (-30.0)) / 30.0) * (p75_2 - p25_2);
} else if (target_fader_dB_2 >= -60.0) {
    target_mm_2 = p05_2 + ((target_fader_dB_2 - (-60.0)) / 30.0) * (p25_2 - p05_2);
} else {
    target_mm_2 = 0.0; 
}

Fader_Target_mm_2 = target_mm_2;
Gain_Target_deg_2 = target_gain_dB_2 * gain_deg_per_dB;