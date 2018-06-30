#include "rfm69.h"

volatile enum Rfm69_Mode rfm69_current_mode;
volatile bool is_rfm69hw = false;
volatile uint8_t rfm69_power_level = 31;

void rfm69_disable_high_power_regs()
{
    rfm69_write_reg(REG_TESTPA1, 0x55);
    rfm69_write_reg(REG_TESTPA2, 0x70);
}

void rfm69_enable_high_power_regs()
{
    rfm69_write_reg(REG_TESTPA1, 0x5D);
    rfm69_write_reg(REG_TESTPA2, 0x7C);
}

void rfm69_init(uint16_t module_freq, uint8_t network_id)
{
    const uint8_t CONFIG[][2] =
    {
        /* The sequencer wakes up each sub block in a predefined and optimized sequence automatically when switching from one mode to another.  Let's use it. */
        /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
        /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
        /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_4800},
        /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_4800},
        /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_50000},
        /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_50000},

        //* 0x07 */ { REG_FRFMSB, RF_FRFMSB_433},
        //* 0x08 */ { REG_FRFMID, RF_FRFMID_433},
        //* 0x09 */ { REG_FRFLSB, RF_FRFLSB_433},
        
        /* 0x07 */ { REG_FRFMSB, (uint8_t) (module_freq == RF_315MHZ ? RF_FRFMSB_315 : (module_freq == RF_433MHZ ? RF_FRFMSB_433 : (module_freq == RF_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
        /* 0x08 */ { REG_FRFMID, (uint8_t) (module_freq == RF_315MHZ ? RF_FRFMID_315 : (module_freq == RF_433MHZ ? RF_FRFMID_433 : (module_freq == RF_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
        /* 0x09 */ { REG_FRFLSB, (uint8_t) (module_freq == RF_315MHZ ? RF_FRFLSB_315 : (module_freq == RF_433MHZ ? RF_FRFLSB_433 : (module_freq == RF_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },

        // looks like PA1 and PA2 are not implemented on RFM69W, hence the max output power is 13dBm
        // +17dBm and +20dBm are possible on RFM69HW
        // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
        // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
        // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
        ///* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
        ///* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 }, // over current protection (default is 95mA)
            
        // 3 preamble bytes + 2 sync word bytes + 4 payload bytes + 2 crc bytes == 11 bytes.
        // 2.4kbps == 2.4 bits/ms.  88 bits / 2.4 bits per ms == 36.67 ms per packet
        // 4.8kbps == 4.8 bits/ms.  88 bits / 4.8 bits per ms == 18.33 ms per packet
        
        // 2 preamble bytes + 1 sync word byte + 4 payload bytes + 2 crc bytes == 9 bytes
        // 2.4kbps == 2.4 bits/ms.  72 bits / 2.4 bits per ms == 30 ms per packet
        // 4.8kbps == 4.8 bits/ms.  72 bits / 4.8 bits per ms == 15 ms per packet

        /* 0x18 */ /* { REG_LNA, RF_LNA_ZIN_50 } */ // Impedance - test 50 ohms and 200 ohms to see which gives better results
        // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
        /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate must be < 2 * RxBw).  These settings give 125khz in FSK and 62.5k in OOK
        //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
        /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
        /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
        /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
        /* 0x29 */ { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
        // /* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
        /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
        /* 0x2F */ { REG_SYNCVALUE1, 0x3D },
        /* 0x30 */ { REG_SYNCVALUE2, network_id },
        /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_FIXED | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
        /* 0x38 */ { REG_PAYLOADLENGTH, 4 },
        ///* 0x39 */ { REG_NODEADRS, node_id }, // turned off because we're not using address filtering
        /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
        /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent) TODO:  Calculate proper value here
        /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0?
        {255, 0}
    };
    
    master_spi_init();
    
    select_slave(SS_PORT, SS_PIN);
    
    // Let's make sure we're talking to a live RFM69 module by writing some test sync values.
    start_timer2_timeout(TIMER2_OVERFLOWS_BEFORE_RFM_INIT_TIMEOUT);
    while (rfm69_read_reg(REG_SYNCVALUE1) != 0xAA && !timer2_timeout_complete())
    {
        rfm69_write_reg(REG_SYNCVALUE1, 0xAA);
    }
    
    start_timer2_timeout(TIMER2_OVERFLOWS_BEFORE_RFM_INIT_TIMEOUT);
    while (rfm69_read_reg(REG_SYNCVALUE1) != 0x55 && !timer2_timeout_complete())
    {
        rfm69_write_reg(REG_SYNCVALUE1, 0x55);
    }
    
    // Now to write our actual configuration
    for(uint8_t i = 0; CONFIG[i][0] != 255; i++)
    {
        rfm69_write_reg(CONFIG[i][0], CONFIG[i][1]);
    }
    
    // Encryption is persistent between resets and can trip you up during debugging.
    // Disable it during initialization so we always start from a known state.
    rfm69_set_encryption(RFM69_NO_ENCRYPTION_VAL);
    rfm69_init_high_power(is_rfm69hw);  
    rfm69_set_mode(RFM69_MODE_STANDBY);
    
    // Wait for our mode change to be ready
    start_timer2_timeout(TIMER2_OVERFLOWS_BEFORE_RFM_INIT_TIMEOUT);
    while (((rfm69_read_reg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) && !timer2_timeout_complete());
}

/**
 * Initializes high power capabilities for RFM69HW.  Do not pass true in to this method if you are not
 * using an RFM69HW - transmission will not work if you do so.
 *
 * @param is_rfm69hw - should be true if we are using an RFM69HW module - should be false otherwise
 */
void rfm69_init_high_power(bool is_rfm69hw)
{
    rfm69_write_reg(REG_OCP, is_rfm69hw ? RF_OCP_OFF : RF_OCP_ON);
    if(is_rfm69hw) {
        rfm69_write_reg(REG_PALEVEL, (rfm69_read_reg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
    } else {
        rfm69_write_reg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | rfm69_power_level); // enable P0 only
    }
}

void rfm69_set_encryption(const char* key)
{
    rfm69_set_mode(RFM69_MODE_STANDBY);
    
    if (key != RFM69_NO_ENCRYPTION_VAL) {
        select_slave(SS_PORT, SS_PIN);
        // Let the RFM69 know we want to write to this register with this bitmask
        spi_transceieve(REG_AESKEY1 | RFM69_REG_READ_WRITE_BIT_LOCATION);
        for (uint8_t i = 0; i < 16; i++) {
            spi_transceieve(key[i]);
        }
        unselect_slave(SS_PORT, SS_PIN);
    } else {
        // The LSB bit in REG_PACKETCONFIG2 toggles encryption - 1 for on, 0 for off.  Let's turn it off without modifying any other part of the register.
        rfm69_write_reg(REG_PACKETCONFIG2, (rfm69_read_reg(REG_PACKETCONFIG2) & 0xFE) | 0x00);
    }
}

void rfm69_set_mode(enum Rfm69_Mode new_mode)
{
    if (new_mode == rfm69_current_mode)
    return;

    switch (new_mode)
    {
        case RFM69_MODE_TX:
        rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
        if (is_rfm69hw) rfm69_enable_high_power_regs();
        break;
        
        case RFM69_MODE_RX:
        rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
        if (is_rfm69hw) rfm69_disable_high_power_regs();      
        break;
        
        case RFM69_MODE_SYNTH:
        rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
        break;
        
        case RFM69_MODE_STANDBY:
        rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
        break;
        
        case RFM69_MODE_SLEEP:
        rfm69_write_reg(REG_OPMODE, (rfm69_read_reg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
        break;
        
        default:
        return;
    }
    
    // we are using packet mode, so this check is not really needed
    // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
    while (rfm69_current_mode == RFM69_MODE_SLEEP && (rfm69_read_reg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
    rfm69_current_mode = new_mode;  
}

/**
* Sets the power level for the RFM69 where 0 is the minimum (least powerful transmission) and
* 31 is the maximum power.  Any value over 31 will be treated as 31.  Power level is halved if
* the module is an RFM69HW.
*
* @param power_level - power level to set the RFM69 module to
*/
void rfm69_set_power_level(uint8_t power_level)
{
    rfm69_power_level = (power_level > 31 ? 31 : power_level);
    if(is_rfm69hw) power_level /= 2;
    rfm69_write_reg(REG_PALEVEL, (rfm69_read_reg(REG_PALEVEL) & 0xE0) | rfm69_power_level);
}

void rfm69_write_reg(uint8_t reg_addr, uint8_t value)
{
    select_slave(SS_PORT, SS_PIN);
    
    // Set read/write bit to indicate we're writing, not reading the register
    spi_transceieve(BIT_SET(reg_addr, RFM69_REG_READ_WRITE_BIT_LOCATION));
    spi_transceieve(value);
    
    unselect_slave(SS_PORT, SS_PIN);
}

uint8_t rfm69_read_reg(uint8_t reg_addr)
{
    select_slave(SS_PORT, SS_PIN);
        
    // Clear read/write bit to indicate we're reading, not writing to the register
    spi_transceieve(BIT_CLEAR(reg_addr, RFM69_REG_READ_WRITE_BIT_LOCATION));
    // Send some dummy data to clock out the register value from the RFM69
    uint8_t reg_val = spi_transceieve(0);
        
    unselect_slave(SS_PORT, SS_PIN);
    return reg_val;
}

