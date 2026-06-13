#define DT_DRV_COMPAT zmk_input_processor_pinch_zoom_keys

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>
#include <drivers/input_processor.h>

#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/virtual_key_position.h>

struct pinch_zoom_keys_config {
    uint8_t index;
    uint16_t pinch_code;
    struct zmk_behavior_binding zoom_in;
    struct zmk_behavior_binding zoom_out;
};

struct pinch_zoom_keys_data {
    bool pinch_active;
};

static int invoke_tap(const struct zmk_behavior_binding *binding,
                      struct zmk_input_processor_state *state, uint8_t index) {
    struct zmk_behavior_binding_event event = {
        .position = ZMK_VIRTUAL_KEY_POSITION_BEHAVIOR_INPUT_PROCESSOR(
            state->input_device_index, index),
        .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
    };

    int ret = zmk_behavior_invoke_binding(binding, event, true);
    if (ret < 0) {
        return ret;
    }

    event.timestamp = k_uptime_get();
    return zmk_behavior_invoke_binding(binding, event, false);
}

static int pinch_zoom_keys_handle_event(const struct device *dev, struct input_event *event,
                                        uint32_t param1, uint32_t param2,
                                        struct zmk_input_processor_state *state) {
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);

    const struct pinch_zoom_keys_config *cfg = dev->config;
    struct pinch_zoom_keys_data *data = dev->data;

    if (event->type == INPUT_EV_KEY && event->code == cfg->pinch_code) {
        data->pinch_active = event->value != 0;
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (!data->pinch_active || event->type != INPUT_EV_REL ||
        event->code != INPUT_REL_WHEEL || event->value == 0) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    const struct zmk_behavior_binding *binding =
        event->value > 0 ? &cfg->zoom_in : &cfg->zoom_out;
    int ret = invoke_tap(binding, state, cfg->index);
    if (ret < 0) {
        return ret;
    }

    event->value = 0;
    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api pinch_zoom_keys_driver_api = {
    .handle_event = pinch_zoom_keys_handle_event,
};

#define PINCH_ZOOM_KEYS_INST(n)                                                                   \
    static struct pinch_zoom_keys_data pinch_zoom_keys_data_##n;                                 \
    static const struct pinch_zoom_keys_config pinch_zoom_keys_config_##n = {                    \
        .index = n,                                                                              \
        .pinch_code = DT_INST_PROP_OR(n, pinch_code, INPUT_BTN_7),                               \
        .zoom_in = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                                \
        .zoom_out = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                               \
    };                                                                                           \
    BUILD_ASSERT(DT_INST_PROP_LEN(n, bindings) == 2,                                             \
                 "pinch zoom keys requires exactly two bindings");                              \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &pinch_zoom_keys_data_##n,                              \
                          &pinch_zoom_keys_config_##n, POST_KERNEL,                              \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &pinch_zoom_keys_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PINCH_ZOOM_KEYS_INST)
