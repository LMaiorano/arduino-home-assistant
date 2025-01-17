#include "HASwitch.h"
#ifndef EX_ARDUINOHA_SWITCH

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HASwitch::HASwitch(const char* uniqueId) :
    HABaseDeviceType("switch", uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _currentState(false),
    _commandCallback(nullptr)
{

}

bool HASwitch::setState(const bool state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    if (publishState(state)) {
        _currentState = state;
        return true;
    }

    return false;
}

void HASwitch::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 10); // 10 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAUniqueIdProperty, _uniqueId);
    _serializer->set(HADeviceClassProperty, _class);
    _serializer->set(HAIconProperty, _icon);

    // optional property
    if (_retain) {
        _serializer->set(
            HARetainProperty,
            &_retain,
            HASerializer::BoolPropertyType
        );
    }

    _serializer->set(
        HAOptimisticProperty,
        &_optimistic,
        HASerializer::BoolPropertyType
    );
    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
    _serializer->topic(HACommandTopic);
}

void HASwitch::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();

    if (!_retain) {
        publishState(_currentState);
    }

    subscribeTopic(uniqueId(), HACommandTopic);
}

void HASwitch::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    (void)payload;

    if (_commandCallback && HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        HACommandTopic
    )) {
        bool state = length == strlen_P(HAStateOn);
        _commandCallback(state, this);
    }
}

bool HASwitch::publishState(const bool state)
{
    return publishOnDataTopic(
        HAStateTopic,
        state ? HAStateOn : HAStateOff,
        true,
        true
    );
}

#endif
