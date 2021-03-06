-- Sensor
--
-- This BoundObject represents a Sensor (sensor.h)

require 'dabes.bound_object'

Sensor = BoundObject:extend({
    lib = dab_sensor,

-- Default Configuration

-- Hook Overloads
    realize = function(class, w, h, offset)
        return class.lib.new(w, h, offset)
    end,

-- Forwarded Methods
    _getters = {
        on_static = BoundObject.fwd_func('get_on_static')
    },

    _setters = {
        on_static = BoundObject.readonly
    }
})
