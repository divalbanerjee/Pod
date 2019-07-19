#include "I2CManager.h"

bool I2CManager::open_i2c(int * fd) {
  if ((*fd = open("/dev/i2c-2" , O_RDWR)) < 0) {
    print(LogLevel::LOG_ERROR, "I2C Manager setup failed. Failed to open bus: %s\n", strerror(errno));
    return false;
  }

  return true;
}

bool I2CManager::set_i2c_addr(int fd, int addr) {
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    print(LogLevel::LOG_ERROR, "I2C Manager setup failed. Failed to ioctl: %s\n", strerror(errno));
    return false;
  }

  return true;
}

bool I2CManager::initialize_source() {
  #ifndef BBB
  print(LogLevel::LOG_DEBUG, "I2C Manger setup failed. Not on BBB\n");
  return false;
  #endif

  // Setup this variable which we will re-use
  old_data = std::make_shared<I2CData>();

  if (!open_i2c(&i2c_fd)) {
    set_error_flag(Command::Network_Command_ID::SET_I2C_ERROR, I2CErrors::I2C_SETUP_FAILURE);
    return false;
  }

  if (!set_i2c_addr(i2c_fd, 0x48)) {
    set_error_flag(Command::Network_Command_ID::SET_I2C_ERROR, I2CErrors::I2C_SETUP_FAILURE);
    return false;
  }
  
  print(LogLevel::LOG_DEBUG, "I2C Manger setup successful\n");
  return true;
}

void I2CManager::stop_source() {
  close(i2c_fd);
  print(LogLevel::LOG_DEBUG, "I2C Manger stopped\n");
}

bool I2CManager::single_shot(int fd, int port, int16_t *value) {
  uint8_t writeBuf[3];

  // set config register and start conversion
  // config register is 1
  writeBuf[0] = 1;    

  // bit 15 1 bit for single shot
  // Bits 14-12 input selection
  // 100 ANC0; 101 ANC1; 110 ANC2; 111 ANC3
  // Bits 11-9 Amp gain. Default to 010 here 001 P19
  // Bit 8 Operational mode of the ADS1115.
  // 0 : Continuous conversion mode
  // 1 : Power-down single-shot mode (default)
  // Bits 7-5 data rate to 111 for 860SPS
  // Bits 4-0  comparator functions see spec sheet.
  writeBuf[1] = 0x83;  // 0b10000011; // bits 15-8
  writeBuf[1] = writeBuf[1] | port << 4;  // bits 15-8
  writeBuf[2] = 0xe3;  // 0b11100011; // bits 7-0 

  // begin conversion
  if (write(fd, writeBuf, 3) != 3) {
    print(LogLevel::LOG_ERROR, "I2C Write error. Write config reg. %s\n", strerror(errno));
    return false;
  }

  // wait for conversion complete
  // checking bit 15
  do {
    if (read(fd, writeBuf, 2) != 2) {
      print(LogLevel::LOG_ERROR, "I2C Read error. Read config reg. %s\n", strerror(errno));
      return false;
    }
  } while ((writeBuf[0] & 0x80) == 0);

  // read conversion register -- first write register pointer
  uint8_t readBuf[3];   // conversion register is 0
  readBuf[0] = 0;   // conversion register is 0
  if (write(fd, readBuf, 1) != 1) {
    print(LogLevel::LOG_ERROR, "I2C Write error. Write conversion register. %s\n", strerror(errno));
    return false;
  }
  
  // read 2 bytes
  if (read(fd, readBuf, 2) != 2) {
    print(LogLevel::LOG_ERROR, "I2C Read error. Read conversion error. %s\n", strerror(errno));
    return false;
  }

  // convert display results
  *value = readBuf[0] << 8 | readBuf[1];

  return true;
}

std::shared_ptr<I2CData> I2CManager::refresh() {
  int port;
  int addr;

  if (index == 0) {
    port = ANC0;
    index++;
  } else if (index == 1) {
    port = ANC1;
    index++;
  } else if (index == 2) {
    port = ANC2;
    index++;
  } else {
    port = ANC3;
    index = 0;
  }

  addr = 0x48;

  int64_t a = Utils::microseconds(); 
  int16_t value = 0;
  
  if (!set_i2c_addr(i2c_fd, addr)) {
    Command::set_error_flag(Command::SET_I2C_ERROR, I2C_READ_ERROR); 
    return empty_data();
  }
  if (single_shot(i2c_fd, port, &value)) {
    int64_t b = Utils::microseconds(); 
  } else {
    Command::set_error_flag(Command::SET_I2C_ERROR, I2C_READ_ERROR);
    return empty_data();
  }

  // Update the "old" data with the new reading
  old_data->pressures[index] = value; 

  // duplicate the "old_data" here into the "new_data"
  std::shared_ptr<I2CData> new_data = std::make_shared<I2CData>(*old_data);  
  
  // new_data contains both the new and old readings.
  return new_data;
}

std::shared_ptr<I2CData> I2CManager::refresh_sim() {
  #ifdef SIM
  return SimulatorManager::sim.sim_get_i2c();
  #else
  return empty_data();
  #endif
}

void I2CManager::initialize_sensor_error_configs() {
    if (!(ConfiguratorManager::config.getValue("error_pneumatic_brake_high_over_pressure", error_pneumatic_brake_high_over_pressure) && ConfiguratorManager::config.getValue("error_pneumatic_brake_high_under_pressure", error_pneumatic_brake_high_under_pressure) && ConfiguratorManager::config.getValue("error_pneumatic_brake_regulator_over_pressure", error_pneumatic_brake_regulator_over_pressure) && ConfiguratorManager::config.getValue("error_pneumatic_brake_regulator_under_pressure", error_pneumatic_brake_regulator_under_pressure) && ConfiguratorManager::config.getValue("error_pneumatic_brake_end_over_pressure", error_pneumatic_brake_end_over_pressure) && ConfiguratorManager::config.getValue("error_pneumatic_brake_end_under_pressure", error_pneumatic_brake_end_under_pressure) && ConfiguratorManager::config.getValue("error_battery_box_over_pressure", error_battery_box_over_pressure) && ConfiguratorManager::config.getValue("error_battery_box_under_pressure", error_battery_box_under_pressure))) {
    
    print(LogLevel::LOG_ERROR, "CONFIG FILE ERROR: I2CManager Missing necessary configuration\n");
    exit(1);
  }
}

void I2CManager::check_for_sensor_error(const std::shared_ptr<I2CData> & check_data, E_States state) {
  //auto temp_arr = check_data->temp;
  auto pressure_arr = check_data->pressures;
    if (pressure_arr[0] > error_pneumatic_brake_high_over_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_HIGH_OVER_PRESSURE_ERROR);
    }
    if (pressure_arr[0] < error_pneumatic_brake_high_under_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_HIGH_UNDER_PRESSURE_ERROR);
    }
    if (pressure_arr[1] > error_pneumatic_brake_regulator_over_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_REGULATOR_OVER_PRESSURE_ERROR);
    }
    if (pressure_arr[1] < error_pneumatic_brake_regulator_under_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_REGULATOR_UNDER_PRESSURE_ERROR);
    }
    if (pressure_arr[2] > error_pneumatic_brake_end_over_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_END_OVER_PRESSURE_ERROR);
    }
    if (pressure_arr[2] < error_pneumatic_brake_end_under_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BRAKE_END_UNDER_PRESSURE_ERROR);
    }
    if (pressure_arr[3] > error_battery_box_over_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BATTERY_BOX_OVER_PRESSURE_ERROR);
    }
    if (pressure_arr[3] > error_battery_box_under_pressure) {
        Command::set_error_flag(Command::SET_I2C_ERROR, I2C_BATTERY_BOX_UNDER_PRESSURE_ERROR);
    }
}
