constexpr uint8_t const suit_number = 4;
constexpr uint16_t const num_events = 399;
constexpr uint8_t const choreo[] PROGMEM = { 0xA8, 0x03, 0xFF, 0x68, 0x04, 0x00, 0x91, 0x0D, 0xFF, 0x67, 0x10, 0x00, 0xE4, 0x18, 0xFF, 0xD7, 0x1B, 0x00, 0xE7, 0x1B, 0x04, 0x26, 0x1C, 0x0C, 0x29, 0x1C, 0xEC, 0x55, 0x1C, 0xFF, 0x5F, 0x1C, 0xEC, 0x69, 0x1C, 0xFF, 0x73, 0x1C, 0xEC, 0x79, 0x1C, 0xFF, 0x84, 0x1C, 0xEC, 0x90, 0x1C, 0xFF, 0x9B, 0x1C, 0xEC, 0xB6, 0x1C, 0xFF, 0xD4, 0x1C, 0xEC, 0x09, 0x1D, 0xFF, 0x14, 0x1D, 0xEC, 0x1D, 0x1D, 0xFF, 0x27, 0x1D, 0xEC, 0x2E, 0x1D, 0xFF, 0x38, 0x1D, 0xEC, 0x45, 0x1D, 0xFF, 0x4F, 0x1D, 0xEC, 0x69, 0x1D, 0xFF, 0x87, 0x1D, 0xEC, 0x39, 0x1F, 0xFF, 0x43, 0x1F, 0xEC, 0x4C, 0x1F, 0xFF, 0x57, 0x1F, 0xEC, 0x5D, 0x1F, 0xFF, 0x67, 0x1F, 0xEC, 0x75, 0x1F, 0xFF, 0x7F, 0x1F, 0xEC, 0x99, 0x1F, 0xFF, 0xB8, 0x1F, 0xEC, 0xD9, 0x1F, 0xFF, 0xE3, 0x1F, 0xEC, 0xED, 0x1F, 0xFF, 0xF7, 0x1F, 0xEC, 0xFD, 0x1F, 0xFF, 0x08, 0x20, 0xEC, 0x14, 0x20, 0xFF, 0x1F, 0x20, 0xEC, 0x3A, 0x20, 0xFF, 0x58, 0x20, 0xEC, 0xB9, 0x21, 0xE4, 0xBB, 0x21, 0x00, 0xFB, 0x2B, 0x11, 0x02, 0x2C, 0x13, 0x05, 0x2C, 0x02, 0x09, 0x2C, 0x0E, 0x0C, 0x2C, 0x0C, 0x0F, 0x2C, 0xEC, 0x12, 0x2C, 0xE0, 0x16, 0x2C, 0xF1, 0x18, 0x2C, 0x11, 0x1D, 0x2C, 0x13, 0x20, 0x2C, 0x02, 0x24, 0x2C, 0x0E, 0x26, 0x2C, 0x0C, 0x2A, 0x2C, 0xEC, 0x2D, 0x2C, 0xE0, 0x30, 0x2C, 0xF1, 0x34, 0x2C, 0x11, 0x38, 0x2C, 0x13, 0x3A, 0x2C, 0x02, 0x3E, 0x2C, 0x0E, 0x41, 0x2C, 0x0C, 0x45, 0x2C, 0xEC, 0x47, 0x2C, 0xE0, 0x4B, 0x2C, 0xF1, 0x4F, 0x2C, 0x11, 0x52, 0x2C, 0x13, 0x55, 0x2C, 0x02, 0x59, 0x2C, 0x0E, 0x5B, 0x2C, 0x0C, 0x5F, 0x2C, 0xEC, 0x62, 0x2C, 0xE0, 0x69, 0x2C, 0x00, 0xDE, 0x2C, 0x11, 0xE5, 0x2C, 0x13, 0xE8, 0x2C, 0x02, 0xEC, 0x2C, 0x0E, 0xEE, 0x2C, 0x0C, 0xF2, 0x2C, 0xEC, 0xF5, 0x2C, 0xE0, 0xF8, 0x2C, 0xF1, 0xFC, 0x2C, 0x11, 0x00, 0x2D, 0x13, 0x02, 0x2D, 0x02, 0x06, 0x2D, 0x0E, 0x09, 0x2D, 0x0C, 0x0D, 0x2D, 0xEC, 0x0F, 0x2D, 0xE0, 0x13, 0x2D, 0xF1, 0x17, 0x2D, 0x11, 0x1A, 0x2D, 0x13, 0x1D, 0x2D, 0x02, 0x21, 0x2D, 0x0E, 0x23, 0x2D, 0x0C, 0x27, 0x2D, 0xEC, 0x2A, 0x2D, 0xE0, 0x2E, 0x2D, 0xF1, 0x31, 0x2D, 0x11, 0x35, 0x2D, 0x13, 0x38, 0x2D, 0x02, 0x3B, 0x2D, 0x0E, 0x3E, 0x2D, 0x0C, 0x42, 0x2D, 0xEC, 0x44, 0x2D, 0xE0, 0x4C, 0x2D, 0x00, 0x60, 0x2D, 0xFF, 0x73, 0x2D, 0xFC, 0x74, 0x2D, 0x00, 0x7B, 0x2D, 0xFF, 0x8E, 0x2D, 0x00, 0x99, 0x2D, 0xFF, 0xAD, 0x2D, 0x00, 0xB7, 0x2D, 0xFF, 0xCB, 0x2D, 0x00, 0xD1, 0x2D, 0xFF, 0xE5, 0x2D, 0x00, 0x32, 0x2E, 0xC0, 0x67, 0x2E, 0x33, 0x7C, 0x2E, 0x0C, 0x8E, 0x2E, 0x00, 0x8F, 0x2E, 0xC0, 0xA2, 0x2E, 0x33, 0xE2, 0x2E, 0x00, 0xE3, 0x2E, 0x0C, 0xF7, 0x2E, 0xC0, 0x0B, 0x2F, 0x33, 0x1D, 0x2F, 0x0C, 0x41, 0x2F, 0x00, 0xBC, 0x2F, 0xFF, 0x26, 0x31, 0x00, 0x2D, 0x31, 0xFF, 0x44, 0x31, 0x00, 0x4F, 0x31, 0xFF, 0x66, 0x31, 0x00, 0x70, 0x31, 0xFF, 0x87, 0x31, 0x00, 0x91, 0x31, 0xFF, 0xAC, 0x31, 0x00, 0xB6, 0x31, 0xFF, 0x16, 0x32, 0x00, 0x44, 0x32, 0x20, 0x4D, 0x32, 0x00, 0x76, 0x32, 0x20, 0x7F, 0x32, 0x00, 0xA8, 0x32, 0x20, 0xB1, 0x32, 0x00, 0xDA, 0x32, 0x20, 0xE3, 0x32, 0x00, 0x0A, 0x33, 0x20, 0x82, 0x33, 0xFF, 0xF4, 0x33, 0x00, 0xFD, 0x33, 0xC0, 0x08, 0x34, 0xC2, 0x11, 0x34, 0xCE, 0x1B, 0x34, 0xCF, 0x29, 0x34, 0xFF, 0x44, 0x34, 0xCF, 0x51, 0x34, 0xCE, 0x5A, 0x34, 0xC2, 0x65, 0x34, 0xC0, 0x72, 0x34, 0x00, 0x75, 0x34, 0xFF, 0xED, 0x34, 0x00, 0xEE, 0x34, 0xDF, 0x08, 0x35, 0xFF, 0x09, 0x35, 0xEE, 0x26, 0x35, 0xFD, 0x41, 0x35, 0x3F, 0x5A, 0x35, 0x1F, 0x5B, 0x35, 0xFF, 0xAA, 0x35, 0x3F, 0xAB, 0x35, 0xC0, 0xDC, 0x35, 0x00, 0x5C, 0x36, 0x0C, 0x6F, 0x36, 0xCC, 0x70, 0x36, 0xC0, 0x83, 0x36, 0x33, 0x96, 0x36, 0x00, 0x43, 0x3C, 0xFF, 0xAC, 0x3F, 0x00, 0x7E, 0x40, 0xFF, 0x89, 0x40, 0x00, 0x8F, 0x40, 0xFF, 0x9D, 0x40, 0x00, 0xA2, 0x40, 0xFF, 0xAC, 0x40, 0x00, 0xB3, 0x40, 0xFF, 0xC7, 0x40, 0x00, 0xD1, 0x40, 0xFF, 0xE4, 0x40, 0x00, 0xEE, 0x40, 0xFF, 0x01, 0x41, 0x00, 0x0A, 0x41, 0xFF, 0x1E, 0x41, 0x00, 0x24, 0x41, 0xFF, 0x3B, 0x41, 0x00, 0x45, 0x41, 0xFF, 0x56, 0x41, 0x00, 0x5F, 0x41, 0xFF, 0x70, 0x41, 0x00, 0x7A, 0x41, 0xFF, 0x99, 0x41, 0x00, 0xA4, 0x41, 0xFF, 0xAA, 0x41, 0x00, 0xB2, 0x41, 0xFF, 0xBC, 0x41, 0x00, 0x3A, 0x42, 0xFF, 0x44, 0x42, 0x00, 0x4A, 0x42, 0xFF, 0x57, 0x42, 0x00, 0x5D, 0x42, 0xFF, 0x68, 0x42, 0x00, 0x6F, 0x42, 0xFF, 0x83, 0x42, 0x00, 0x8C, 0x42, 0xFF, 0xA0, 0x42, 0x00, 0xAA, 0x42, 0xFF, 0xBD, 0x42, 0x00, 0xC5, 0x42, 0xFF, 0xD8, 0x42, 0x00, 0xE0, 0x42, 0xFF, 0xF7, 0x42, 0x00, 0x00, 0x43, 0xFF, 0x11, 0x43, 0x00, 0x1B, 0x43, 0xFF, 0x2C, 0x43, 0x00, 0x33, 0x43, 0xFF, 0x5B, 0x43, 0x00, 0x66, 0x43, 0xFF, 0x6D, 0x43, 0x00, 0x76, 0x43, 0xFF, 0x7D, 0x43, 0x00, 0x9A, 0x43, 0xFF, 0xA1, 0x43, 0x00, 0xAC, 0x43, 0xFF, 0xB3, 0x43, 0x00, 0xB6, 0x43, 0xFF, 0xBD, 0x43, 0x00, 0xC8, 0x43, 0xFF, 0xCF, 0x43, 0x00, 0xD2, 0x43, 0xFF, 0xDA, 0x43, 0x00, 0xEF, 0x43, 0xFF, 0xF6, 0x43, 0x00, 0x19, 0x44, 0xFF, 0x21, 0x44, 0x00, 0x26, 0x44, 0xFF, 0x2D, 0x44, 0x00, 0x42, 0x44, 0xFF, 0x49, 0x44, 0x00, 0x53, 0x44, 0xFF, 0x5B, 0x44, 0x00, 0x5F, 0x44, 0xFF, 0x66, 0x44, 0x00, 0x7A, 0x44, 0xFF, 0x82, 0x44, 0x00, 0x8B, 0x44, 0xFF, 0x92, 0x44, 0x00, 0x97, 0x44, 0xFF, 0x9E, 0x44, 0x00, 0xA8, 0x44, 0xFF, 0xAF, 0x44, 0x00, 0xB4, 0x44, 0xFF, 0xBB, 0x44, 0x00, 0xCE, 0x44, 0xFF, 0xD6, 0x44, 0x00, 0xDF, 0x44, 0xFF, 0xE6, 0x44, 0x00, 0xEC, 0x44, 0xFF, 0xF3, 0x44, 0x00, 0x08, 0x45, 0xFF, 0x0F, 0x45, 0x00, 0x19, 0x45, 0xFF, 0x21, 0x45, 0x00, 0x34, 0x45, 0xFF, 0x3B, 0x45, 0x00, 0x40, 0x45, 0xFF, 0x47, 0x45, 0x00, 0x50, 0x45, 0xFF, 0x57, 0x45, 0x00, 0x5C, 0x45, 0xFF, 0x63, 0x45, 0x00, 0x6D, 0x45, 0xFF, 0x74, 0x45, 0x00, 0x78, 0x45, 0xFF, 0x7F, 0x45, 0x00, 0x89, 0x45, 0xFF, 0x90, 0x45, 0x00, 0x94, 0x45, 0xFF, 0x9C, 0x45, 0x00, 0xAF, 0x45, 0xFF, 0xB7, 0x45, 0x00, 0xEC, 0x45, 0xFF, 0x04, 0x46, 0x00, 0x1A, 0x46, 0xFF, 0x32, 0x46, 0x00, 0x49, 0x46, 0xFF, 0x60, 0x46, 0x00, 0x85, 0x46, 0x40, 0x8B, 0x46, 0x44, 0x8E, 0x46, 0x04, 0x92, 0x46, 0x37, 0x95, 0x46, 0x33, 0x99, 0x46, 0xBB, 0x9C, 0x46, 0x88, 0xA2, 0x46, 0x00, 0xC8, 0x46, 0xFF, 0x1C, 0x47, 0x00, 0x32, 0x47, 0xC0, 0x39, 0x47, 0xC2, 0x43, 0x47, 0xCE, 0x4D, 0x47, 0xDF, 0x5A, 0x47, 0xFF, 0x83, 0x47, 0x22, 0x84, 0x47, 0x00, 0x93, 0x47, 0xFF, 0xEE, 0x47, 0x00, 0xFD, 0x47, 0xC0, 0x04, 0x48, 0xC2, 0x0E, 0x48, 0xCE, 0x18, 0x48, 0xDF, 0x26, 0x48, 0xFF, 0x62, 0x48, 0x33, 0x63, 0x48, 0x00, 0x72, 0x48, 0xFF, 0xC6, 0x48, 0x00, 0xE0, 0x48, 0xC0, 0xE6, 0x48, 0xC2, 0xF1, 0x48, 0xCE, 0xFB, 0x48, 0xDF, 0x08, 0x49, 0xFF, 0x9F, 0x49, 0x00, 0x70, 0x4A, 0xFF, 0x85, 0x4A, 0x00, 0x1A, 0x4B, 0xFF, 0x2F, 0x4B, 0x00, 0x25, 0x4C, 0xFF, 0x39, 0x4C, 0x00, 0xE9, 0x4C, 0xFF, 0xC5, 0x4D, 0xFF, 0xD0, 0x4D, 0xCF, 0xDA, 0x4D, 0xCE, 0xE3, 0x4D, 0xCC, 0xEE, 0x4D, 0xC0, 0xFB, 0x4D, 0x00, 0x4E, 0x53, 0x20, 0x63, 0x53, 0x00, 0x74, 0x54, 0xFF, 0x2F, 0x55, 0x00, 0x8E, 0x59, 0xC0, 0xA5, 0x59, 0x00, 0xA6, 0x59, 0x33, 0xAF, 0x59, 0x3F, 0xB0, 0x59, 0x0C, 0xBE, 0x59, 0xC0, 0xE6, 0x59, 0x33, 0xFF, 0x59, 0x00, 0x01, 0x5A, 0xC0, 0x13, 0x5A, 0xF3, 0x14, 0x5A, 0x33, 0x1D, 0x5A, 0x0C, 0x2B, 0x5A, 0x00, 0x7E, 0x5A, 0xFF, 0xB0, 0x5A, 0x00, 0xE9, 0x5A, 0xFF, 0x1B, 0x5B, 0x00, 0x57, 0x5B, 0xFF, 0x78, 0x5B, 0x00, 0x8C, 0x5B, 0xFD, 0x93, 0x5B, 0xCC, 0x96, 0x5B, 0xFD, 0x9A, 0x5B, 0xCC, 0x9F, 0x5B, 0xFD, 0xA4, 0x5B, 0xCC, 0xA8, 0x5B, 0xFD, 0xAD, 0x5B, 0xCC, 0xB1, 0x5B, 0x00, 0x25, 0x5C, 0x13, 0x2F, 0x5C, 0x00, 0x44, 0x5C, 0xC0, 0x4D, 0x5C, 0x00, 0x89, 0x5C, 0xC0, 0x93, 0x5C, 0x00, 0xA8, 0x5C, 0x13, 0xB1, 0x5C, 0x00, 0xD0, 0x5D, 0x3F, 0x23, 0x62, 0x00, 0x81, 0x63, 0xFF, 0x8F, 0x65, 0x00 };