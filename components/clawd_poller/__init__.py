import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components.esp32 import add_idf_sdkconfig_option
from esphome.const import CONF_ID, CONF_TRIGGER_ID

CODEOWNERS = ["@eldios"]
DEPENDENCIES = ["network"]

clawd_poller_ns = cg.esphome_ns.namespace("clawd_poller")
ClawdPoller = clawd_poller_ns.class_("ClawdPoller", cg.Component)
PollResultTrigger = clawd_poller_ns.class_(
    "PollResultTrigger",
    automation.Trigger.template(
        cg.float_, cg.float_, cg.int64, cg.int64, cg.std_string, cg.int_
    ),
)
PollAction = clawd_poller_ns.class_("PollAction", automation.Action)

CONF_TOKEN = "token"
CONF_PROBE_MODEL = "probe_model"
CONF_ON_RESULT = "on_result"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ClawdPoller),
        cv.Required(CONF_TOKEN): cv.string,
        cv.Optional(CONF_PROBE_MODEL, default="claude-haiku-4-5-20251001"): cv.string,
        cv.Optional(CONF_ON_RESULT): automation.validate_automation(
            {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PollResultTrigger)}
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_token(config[CONF_TOKEN]))
    cg.add(var.set_probe_model(config[CONF_PROBE_MODEL]))
    add_idf_sdkconfig_option("CONFIG_MBEDTLS_CERTIFICATE_BUNDLE", True)
    for conf in config.get(CONF_ON_RESULT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger,
            [
                (cg.float_, "u5"),
                (cg.float_, "u7"),
                (cg.int64, "r5"),
                (cg.int64, "r7"),
                (cg.std_string, "status"),
                (cg.int_, "code"),
            ],
            conf,
        )


@automation.register_action(
    "clawd_poller.poll",
    PollAction,
    automation.maybe_simple_id({cv.GenerateID(): cv.use_id(ClawdPoller)}),
)
async def clawd_poller_poll_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
