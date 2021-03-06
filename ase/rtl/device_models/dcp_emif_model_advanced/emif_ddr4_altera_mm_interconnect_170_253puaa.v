// emif_ddr4_altera_mm_interconnect_170_253puaa.v

// This file was auto-generated from altera_mm_interconnect_hw.tcl.  If you edit it your changes
// will probably be lost.
//
// Generated using ACDS version 17.0 290

`timescale 1 ps / 1 ps
module emif_ddr4_altera_mm_interconnect_170_253puaa (
		input  wire [25:0]  ddr4b_bridge_m0_address,                        //                          ddr4b_bridge_m0.address
		output wire         ddr4b_bridge_m0_waitrequest,                    //                                         .waitrequest
		input  wire [6:0]   ddr4b_bridge_m0_burstcount,                     //                                         .burstcount
		input  wire [63:0]  ddr4b_bridge_m0_byteenable,                     //                                         .byteenable
		input  wire         ddr4b_bridge_m0_read,                           //                                         .read
		output wire [511:0] ddr4b_bridge_m0_readdata,                       //                                         .readdata
		output wire         ddr4b_bridge_m0_readdatavalid,                  //                                         .readdatavalid
		input  wire         ddr4b_bridge_m0_write,                          //                                         .write
		input  wire [511:0] ddr4b_bridge_m0_writedata,                      //                                         .writedata
		input  wire         ddr4b_bridge_m0_debugaccess,                    //                                         .debugaccess
		input  wire         ddr4b_bridge_reset_reset_bridge_in_reset_reset, // ddr4b_bridge_reset_reset_bridge_in_reset.reset
		output wire [25:0]  ddr4b_ctrl_amm_0_address,                       //                         ddr4b_ctrl_amm_0.address
		output wire         ddr4b_ctrl_amm_0_write,                         //                                         .write
		output wire         ddr4b_ctrl_amm_0_read,                          //                                         .read
		input  wire [511:0] ddr4b_ctrl_amm_0_readdata,                      //                                         .readdata
		output wire [511:0] ddr4b_ctrl_amm_0_writedata,                     //                                         .writedata
		output wire [6:0]   ddr4b_ctrl_amm_0_burstcount,                    //                                         .burstcount
		output wire [63:0]  ddr4b_ctrl_amm_0_byteenable,                    //                                         .byteenable
		input  wire         ddr4b_ctrl_amm_0_readdatavalid,                 //                                         .readdatavalid
		input  wire         ddr4b_ctrl_amm_0_waitrequest,                   //                                         .waitrequest
		input  wire         ddr4b_emif_usr_clk_clk                          //                       ddr4b_emif_usr_clk.clk
	);

	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_waitrequest;   // ddr4b_ctrl_amm_0_translator:uav_waitrequest -> ddr4b_bridge_m0_translator:uav_waitrequest
	wire  [511:0] ddr4b_bridge_m0_translator_avalon_universal_master_0_readdata;      // ddr4b_ctrl_amm_0_translator:uav_readdata -> ddr4b_bridge_m0_translator:uav_readdata
	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_debugaccess;   // ddr4b_bridge_m0_translator:uav_debugaccess -> ddr4b_ctrl_amm_0_translator:uav_debugaccess
	wire   [31:0] ddr4b_bridge_m0_translator_avalon_universal_master_0_address;       // ddr4b_bridge_m0_translator:uav_address -> ddr4b_ctrl_amm_0_translator:uav_address
	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_read;          // ddr4b_bridge_m0_translator:uav_read -> ddr4b_ctrl_amm_0_translator:uav_read
	wire   [63:0] ddr4b_bridge_m0_translator_avalon_universal_master_0_byteenable;    // ddr4b_bridge_m0_translator:uav_byteenable -> ddr4b_ctrl_amm_0_translator:uav_byteenable
	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_readdatavalid; // ddr4b_ctrl_amm_0_translator:uav_readdatavalid -> ddr4b_bridge_m0_translator:uav_readdatavalid
	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_lock;          // ddr4b_bridge_m0_translator:uav_lock -> ddr4b_ctrl_amm_0_translator:uav_lock
	wire          ddr4b_bridge_m0_translator_avalon_universal_master_0_write;         // ddr4b_bridge_m0_translator:uav_write -> ddr4b_ctrl_amm_0_translator:uav_write
	wire  [511:0] ddr4b_bridge_m0_translator_avalon_universal_master_0_writedata;     // ddr4b_bridge_m0_translator:uav_writedata -> ddr4b_ctrl_amm_0_translator:uav_writedata
	wire   [12:0] ddr4b_bridge_m0_translator_avalon_universal_master_0_burstcount;    // ddr4b_bridge_m0_translator:uav_burstcount -> ddr4b_ctrl_amm_0_translator:uav_burstcount

	altera_merlin_master_translator #(
		.AV_ADDRESS_W                (26),
		.AV_DATA_W                   (512),
		.AV_BURSTCOUNT_W             (7),
		.AV_BYTEENABLE_W             (64),
		.UAV_ADDRESS_W               (32),
		.UAV_BURSTCOUNT_W            (13),
		.USE_READ                    (1),
		.USE_WRITE                   (1),
		.USE_BEGINBURSTTRANSFER      (0),
		.USE_BEGINTRANSFER           (0),
		.USE_CHIPSELECT              (0),
		.USE_BURSTCOUNT              (1),
		.USE_READDATAVALID           (1),
		.USE_WAITREQUEST             (1),
		.USE_READRESPONSE            (0),
		.USE_WRITERESPONSE           (0),
		.AV_SYMBOLS_PER_WORD         (64),
		.AV_ADDRESS_SYMBOLS          (0),
		.AV_BURSTCOUNT_SYMBOLS       (0),
		.AV_CONSTANT_BURST_BEHAVIOR  (1),
		.UAV_CONSTANT_BURST_BEHAVIOR (1),
		.AV_LINEWRAPBURSTS           (0),
		.AV_REGISTERINCOMINGSIGNALS  (0)
	) ddr4b_bridge_m0_translator (
		.clk                    (ddr4b_emif_usr_clk_clk),                                             //                       clk.clk
		.reset                  (ddr4b_bridge_reset_reset_bridge_in_reset_reset),                     //                     reset.reset
		.uav_address            (ddr4b_bridge_m0_translator_avalon_universal_master_0_address),       // avalon_universal_master_0.address
		.uav_burstcount         (ddr4b_bridge_m0_translator_avalon_universal_master_0_burstcount),    //                          .burstcount
		.uav_read               (ddr4b_bridge_m0_translator_avalon_universal_master_0_read),          //                          .read
		.uav_write              (ddr4b_bridge_m0_translator_avalon_universal_master_0_write),         //                          .write
		.uav_waitrequest        (ddr4b_bridge_m0_translator_avalon_universal_master_0_waitrequest),   //                          .waitrequest
		.uav_readdatavalid      (ddr4b_bridge_m0_translator_avalon_universal_master_0_readdatavalid), //                          .readdatavalid
		.uav_byteenable         (ddr4b_bridge_m0_translator_avalon_universal_master_0_byteenable),    //                          .byteenable
		.uav_readdata           (ddr4b_bridge_m0_translator_avalon_universal_master_0_readdata),      //                          .readdata
		.uav_writedata          (ddr4b_bridge_m0_translator_avalon_universal_master_0_writedata),     //                          .writedata
		.uav_lock               (ddr4b_bridge_m0_translator_avalon_universal_master_0_lock),          //                          .lock
		.uav_debugaccess        (ddr4b_bridge_m0_translator_avalon_universal_master_0_debugaccess),   //                          .debugaccess
		.av_address             (ddr4b_bridge_m0_address),                                            //      avalon_anti_master_0.address
		.av_waitrequest         (ddr4b_bridge_m0_waitrequest),                                        //                          .waitrequest
		.av_burstcount          (ddr4b_bridge_m0_burstcount),                                         //                          .burstcount
		.av_byteenable          (ddr4b_bridge_m0_byteenable),                                         //                          .byteenable
		.av_read                (ddr4b_bridge_m0_read),                                               //                          .read
		.av_readdata            (ddr4b_bridge_m0_readdata),                                           //                          .readdata
		.av_readdatavalid       (ddr4b_bridge_m0_readdatavalid),                                      //                          .readdatavalid
		.av_write               (ddr4b_bridge_m0_write),                                              //                          .write
		.av_writedata           (ddr4b_bridge_m0_writedata),                                          //                          .writedata
		.av_debugaccess         (ddr4b_bridge_m0_debugaccess),                                        //                          .debugaccess
		.av_beginbursttransfer  (1'b0),                                                               //               (terminated)
		.av_begintransfer       (1'b0),                                                               //               (terminated)
		.av_chipselect          (1'b0),                                                               //               (terminated)
		.av_lock                (1'b0),                                                               //               (terminated)
		.uav_clken              (),                                                                   //               (terminated)
		.av_clken               (1'b1),                                                               //               (terminated)
		.uav_response           (2'b00),                                                              //               (terminated)
		.av_response            (),                                                                   //               (terminated)
		.uav_writeresponsevalid (1'b0),                                                               //               (terminated)
		.av_writeresponsevalid  ()                                                                    //               (terminated)
	);

	altera_merlin_slave_translator #(
		.AV_ADDRESS_W                   (26),
		.AV_DATA_W                      (512),
		.UAV_DATA_W                     (512),
		.AV_BURSTCOUNT_W                (7),
		.AV_BYTEENABLE_W                (64),
		.UAV_BYTEENABLE_W               (64),
		.UAV_ADDRESS_W                  (32),
		.UAV_BURSTCOUNT_W               (13),
		.AV_READLATENCY                 (0),
		.USE_READDATAVALID              (1),
		.USE_WAITREQUEST                (1),
		.USE_UAV_CLKEN                  (0),
		.USE_READRESPONSE               (0),
		.USE_WRITERESPONSE              (0),
		.AV_SYMBOLS_PER_WORD            (64),
		.AV_ADDRESS_SYMBOLS             (0),
		.AV_BURSTCOUNT_SYMBOLS          (0),
		.AV_CONSTANT_BURST_BEHAVIOR     (0),
		.UAV_CONSTANT_BURST_BEHAVIOR    (0),
		.AV_REQUIRE_UNALIGNED_ADDRESSES (0),
		.CHIPSELECT_THROUGH_READLATENCY (0),
		.AV_READ_WAIT_CYCLES            (1),
		.AV_WRITE_WAIT_CYCLES           (0),
		.AV_SETUP_WAIT_CYCLES           (0),
		.AV_DATA_HOLD_CYCLES            (0)
	) ddr4b_ctrl_amm_0_translator (
		.clk                    (ddr4b_emif_usr_clk_clk),                                             //                      clk.clk
		.reset                  (ddr4b_bridge_reset_reset_bridge_in_reset_reset),                     //                    reset.reset
		.uav_address            (ddr4b_bridge_m0_translator_avalon_universal_master_0_address),       // avalon_universal_slave_0.address
		.uav_burstcount         (ddr4b_bridge_m0_translator_avalon_universal_master_0_burstcount),    //                         .burstcount
		.uav_read               (ddr4b_bridge_m0_translator_avalon_universal_master_0_read),          //                         .read
		.uav_write              (ddr4b_bridge_m0_translator_avalon_universal_master_0_write),         //                         .write
		.uav_waitrequest        (ddr4b_bridge_m0_translator_avalon_universal_master_0_waitrequest),   //                         .waitrequest
		.uav_readdatavalid      (ddr4b_bridge_m0_translator_avalon_universal_master_0_readdatavalid), //                         .readdatavalid
		.uav_byteenable         (ddr4b_bridge_m0_translator_avalon_universal_master_0_byteenable),    //                         .byteenable
		.uav_readdata           (ddr4b_bridge_m0_translator_avalon_universal_master_0_readdata),      //                         .readdata
		.uav_writedata          (ddr4b_bridge_m0_translator_avalon_universal_master_0_writedata),     //                         .writedata
		.uav_lock               (ddr4b_bridge_m0_translator_avalon_universal_master_0_lock),          //                         .lock
		.uav_debugaccess        (ddr4b_bridge_m0_translator_avalon_universal_master_0_debugaccess),   //                         .debugaccess
		.av_address             (ddr4b_ctrl_amm_0_address),                                           //      avalon_anti_slave_0.address
		.av_write               (ddr4b_ctrl_amm_0_write),                                             //                         .write
		.av_read                (ddr4b_ctrl_amm_0_read),                                              //                         .read
		.av_readdata            (ddr4b_ctrl_amm_0_readdata),                                          //                         .readdata
		.av_writedata           (ddr4b_ctrl_amm_0_writedata),                                         //                         .writedata
		.av_burstcount          (ddr4b_ctrl_amm_0_burstcount),                                        //                         .burstcount
		.av_byteenable          (ddr4b_ctrl_amm_0_byteenable),                                        //                         .byteenable
		.av_readdatavalid       (ddr4b_ctrl_amm_0_readdatavalid),                                     //                         .readdatavalid
		.av_waitrequest         (ddr4b_ctrl_amm_0_waitrequest),                                       //                         .waitrequest
		.av_begintransfer       (),                                                                   //              (terminated)
		.av_beginbursttransfer  (),                                                                   //              (terminated)
		.av_writebyteenable     (),                                                                   //              (terminated)
		.av_lock                (),                                                                   //              (terminated)
		.av_chipselect          (),                                                                   //              (terminated)
		.av_clken               (),                                                                   //              (terminated)
		.uav_clken              (1'b0),                                                               //              (terminated)
		.av_debugaccess         (),                                                                   //              (terminated)
		.av_outputenable        (),                                                                   //              (terminated)
		.uav_response           (),                                                                   //              (terminated)
		.av_response            (2'b00),                                                              //              (terminated)
		.uav_writeresponsevalid (),                                                                   //              (terminated)
		.av_writeresponsevalid  (1'b0)                                                                //              (terminated)
	);

endmodule
