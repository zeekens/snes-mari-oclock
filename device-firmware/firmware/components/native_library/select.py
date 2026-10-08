import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from . import ns,NativeLibrary
NativeSelect=ns.class_('NativeSelect',select.Select,cg.Component)
CONFIG_SCHEMA=select.select_schema(NativeSelect).extend({cv.Required('native_library_id'):cv.use_id(NativeLibrary),cv.Optional('library',default=False):cv.boolean}).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var=cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var,config)
    await select.register_select(var,config,options=['Classic'])
    cg.add(var.set_parent(await cg.get_variable(config['native_library_id']),config['library']))
