TOPIC_MOVE = "UI_receive";
TOPIC_BATTERY = "UI_charging";

function startConnect(){
    clientID = "clientID - "+parseInt(Math.random() * 100);

    host = "192.168.30.8";
    port = "9001";
    userId  = "avatar"; 
    passwordId = "avatar";  

    client = new Paho.MQTT.Client(host, Number(port), clientID);
    client.onMessageArrived = onMessageArrived;
    client.connect({
        onSuccess: onConnect,
        userName: userId,
        password: passwordId
    });
}

function onConnect(){
    client.subscribe(TOPIC_MOVE);
    client.subscribe(TOPIC_BATTERY);
}


function onMessageArrived(message){
    if (message.destinationName == TOPIC_BATTERY) {
        if (parseFloat(message.payloadString == 4.8)) {
            document.getElementById("batterypercentagetext").innerHTML = "Charging";
        } else {
            document.getElementById("batterypercentagetext").innerHTML = batteryPer(message.payloadString) + "% Charged";
            batteryFill(batteryPer(message.payloadString));
        }
    }
}


//forward button
function buttonForward(){
    var message = new Paho.MQTT.Message("straight");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//right button
function buttonRight(){
    var message = new Paho.MQTT.Message("right");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//left button
function buttonLeft(){
    var message = new Paho.MQTT.Message("left");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//back button
function buttonBack(){
    var message = new Paho.MQTT.Message("reverse");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//stop button
function buttonStop(){
    message = new Paho.MQTT.Message("SM");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//go to charger button
function toCharger(){
    message = new Paho.MQTT.Message("CD");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//start autonomous system button
function systemOn(){
    message = new Paho.MQTT.Message("startA");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}

//stop autonomous system button
function systemOff(){
    message = new Paho.MQTT.Message("stopA");
    message.destinationName = TOPIC_MOVE;
    client.send(message);
}


//battery calculations
function batteryPer(voltage){
    //96% --> 66%
    //4.8 = charging
    var min = 2.6;
    var max = 4.2;
    var voltage_diff = voltage - min;
    var percent = Math.trunc((voltage_diff / (max - min)) * 100);

    return percent;
}


//battery fill adjustments
function batteryFill(p) {
    //set color
    if (p < 20) { //red
        document.getElementById("batterypercentageblock").style.backgroundColor= "#fe0a00";
    } else { //green
        document.getElementById("batterypercentageblock").style.backgroundColor= "#27b808";
    }

    //breakpoints
    var tablet = 991;
    var mobile_landscape = 767;
    var mobile = 479;
   
    //max, in pixels
    var max_desktop = 138;
    var max_tablet = 137;
    var max_mobile_landscape = 95;
    var max_mobile = 66;

    //rescale
    var scale_desktop = max_desktop * (p / 100);
    var scale_tablet = max_tablet * (p / 100);
    var scale_mobile_landscape = max_mobile_landscape * (p / 100);
    var scale_mobile = max_mobile * (p / 100);

    //assign correct width
    var windowWidth = window.innerWidth;

    if (windowWidth > tablet) { //desktop
        document.getElementById("batterypercentagetext").style.width = scale_desktop;
    } else if (windowWidth > mobile_landscape && windowWidth <= tablet) { //tablet
        document.getElementById("batterypercentagetext").style.width = scale_tablet;
    } else if (windowWidth > mobile && windowWidth <= mobile_landscape) { //mobile landscape
        document.getElementById("batterypercentagetext").style.width = scale_mobile_landscape;
    } else { //mobile
        document.getElementById("batterypercentagetext").style.width = scale_mobile;
    }
}