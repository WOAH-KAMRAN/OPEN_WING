export const MessageIds = {
  ATTITUDE: 0x01,
  HEADING: 0x02,
  GPS: 0x03,
  RC_CHANNELS: 0x04,
  FLIGHT_MODE: 0x05,
  PID_VALUES: 0x06,
  STATUS: 0x07,
  BATTERY: 0x08,
  SET_PID: 0x81,
  GET_PID: 0x82,
  SET_MODE: 0x83,
  ARM_DISARM: 0x84,
  REBOOT: 0x85
};

export const FlightModes = {
  MANUAL: 0,
  STABILIZE: 1,
  AUTO: 2,
  RTL: 3
};

export const GpsFixTypes = {
  NONE: 0,
  TWO_D: 1,
  THREE_D: 2,
  DGPS: 3
};

export const MessageSizes = {
  [MessageIds.ATTITUDE]: 12,
  [MessageIds.HEADING]: 4,
  [MessageIds.GPS]: 16,
  [MessageIds.RC_CHANNELS]: 16,
  [MessageIds.FLIGHT_MODE]: 2,
  [MessageIds.PID_VALUES]: 36,
  [MessageIds.STATUS]: 4,
  [MessageIds.BATTERY]: 5,
  [MessageIds.SET_PID]: 13,
  [MessageIds.GET_PID]: 1,
  [MessageIds.SET_MODE]: 1,
  [MessageIds.ARM_DISARM]: 1,
  [MessageIds.REBOOT]: 0
};
