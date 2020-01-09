#include "scpi-def.h"

static scpi_result_t DMM_Reset(scpi_t * context) {
    log_d("*RST\r\n"); /* debug command name */

    if (dps_lock(OFF) != 0)
        return SCPI_RES_ERR;

    if (dps_power(OFF) != 0)
        return SCPI_RES_ERR;

    if (dps_change_screen(SCREEN_MAIN) != 0)
        return SCPI_RES_ERR;
    
    if (dps_brightness(50) != 0)
        return SCPI_RES_ERR;
    
    return SCPI_RES_OK;
}

static scpi_result_t DISP_Brightness(scpi_t * context) {
    int32_t brightness;
    log_d("disp:bright\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamInt32(context, &brightness, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_brightness(brightness) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DISP_Mode(scpi_t * context) {
    int32_t mode;
    log_d("disp:mode\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamInt32(context, &mode, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_change_screen(mode) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DMM_OutputStateQ(scpi_t * context) {
    dps_query_t dps_status;

    log_d("outp:stat?\r\n"); /* debug command name */

    if (dps_query(&dps_status) == 0) {
        SCPI_ResultInt64(context, dps_status.output_enabled ? 1 : 0);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

static scpi_result_t DMM_OutputState(scpi_t * context) {
    scpi_bool_t output;
    log_d("outp:stat\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamBool(context, &output, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_power(output) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DMM_MeasureVoltageDcQ(scpi_t * context) {
    dps_query_t dps_status;
    log_d("meas:volt:dc\r\n"); /* debug command name */

    if (dps_query(&dps_status) == 0) {
        SCPI_ResultDouble(context, (double) dps_status.v_out / 1000);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

static scpi_result_t DMM_ConfigureVoltageDc(scpi_t * context) {
    double voltage;
    log_d("conf:volt:dc\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &voltage, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_voltage((int)(voltage * 1000)) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DMM_MeasureCurrentDcQ(scpi_t * context) {
    dps_query_t dps_status;
    log_d("meas:volt:dc\r\n"); /* debug command name */

    if (dps_query(&dps_status) == 0) {
        SCPI_ResultDouble(context, (double) dps_status.i_out / 1000);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

static scpi_result_t DMM_ConfigureCurrentDc(scpi_t * context) {
    double current;
    log_d("conf:volt:dc\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &current, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_current((int)(current * 1000)) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DMM_SystemLocal(scpi_t * context) {
    fprintf(stderr, "syst:locl\r\n"); /* debug command name */

    if (dps_lock(OFF) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

static scpi_result_t DMM_SystemRWLock(scpi_t * context) {
    scpi_bool_t lock;
    log_d("syst:rwl\r\n"); /* debug command name */

    if (!SCPI_ParamBool(context, &lock, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (dps_lock(lock) == 0)
        return SCPI_RES_OK;

    return SCPI_RES_ERR;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = DMM_Reset,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = SCPI_CoreTstQ,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    /* {.pattern = "STATus:OPERation?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:EVENt?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:CONDition?", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:ENABle", .callback = scpi_stub_callback,}, */
    /* {.pattern = "STATus:OPERation:ENABle?", .callback = scpi_stub_callback,}, */

    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    /* {.pattern = "STATus:QUEStionable:CONDition?", .callback = scpi_stub_callback,}, */
    {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

    /* Display */
    {.pattern = "DISPlay:BRIGhtness", .callback = DISP_Brightness,},
    {.pattern = "DISPlay:MODE", .callback = DISP_Mode,},

    /* DMM */
    {.pattern = "MEASure:VOLTage[:DC]?", .callback = DMM_MeasureVoltageDcQ,},
    {.pattern = "SOURce:VOLTage[:DC]", .callback = DMM_ConfigureVoltageDc,},
    {.pattern = "MEASure:VOLTage:DC:RATio?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:VOLTage:AC?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:CURRent[:DC]?", .callback = DMM_MeasureCurrentDcQ,},
    {.pattern = "SOURce:CURRent[:DC]", .callback = DMM_ConfigureCurrentDc,},
    {.pattern = "MEASure:CURRent:AC?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:RESistance?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:FRESistance?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:FREQuency?", .callback = SCPI_StubQ,},
    {.pattern = "MEASure:PERiod?", .callback = SCPI_StubQ,},
    {.pattern = "OUTPut[:STATe]", .callback = DMM_OutputState,},
    {.pattern = "OUTPut[:STATe]?", .callback = DMM_OutputStateQ,},

    {.pattern = "SYSTem:COMMunication:TCPIP:CONTROL?", .callback = SCPI_SystemCommTcpipControlQ,},
    {.pattern = "SYSTem:LOCal", .callback = DMM_SystemLocal,},
    {.pattern = "SYSTem:RWLock", .callback = DMM_SystemRWLock,},

    SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
