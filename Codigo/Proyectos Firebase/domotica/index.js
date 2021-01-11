'use strict';

const functions = require('firebase-functions');
const {dialogflow} = require('actions-on-google');
const firebase = require('firebase-admin');

const admin = firebase.initializeApp();
const baseDatos = firebase.database();

const app = dialogflow({debug: true});

/*****************************************
    FUNCIONES CUMPLIMIENTO INTENCIONES
 *****************************************/

app.intent('encender', (conv, {dispositivo, habitacion}) => {
    var respuesta;
    switch(dispositivo){
        case "luz" : respuesta = encenderLuz(habitacion, conv); break;
        case "calefaccion" : respuesta = encenderCalefaccion(); break;
        case "despertador" : respuesta = activarAlarmaDespertador(); break;
    }
    conv.ask(respuesta);
});

app.intent('apagar', (conv, {dispositivo, habitacion}) => {
    var respuesta;
    switch(dispositivo){
        case "luz" : respuesta = apagarLuz(habitacion, conv); break;
        case "calefaccion" : respuesta = apagarCalefaccion(); break;
        case "despertador" : respuesta = desactivarAlarmaDespertador(); break;
    }
    conv.ask(respuesta);
});

app.intent('consultar estado dispositivo', (conv, {dispositivo, habitacion}) => {
    switch(dispositivo){
        case "luz" :
            return consultarEstadoLuz(habitacion, conv).then((respuesta) => {
                conv.ask(respuesta);
              });
              break;
        case "calefaccion" :
            return consultarEstadoCalefaccion().then((respuesta) => {
                conv.ask(respuesta);
              });
              break;
        case "despertador" :
            return consultarEstadoDespertador().then((respuesta) => {
                    conv.ask(respuesta);
              });
              break;
    }
});

app.intent('obtener habitación', (conv, {habitacion}) => {
    var respuesta;
    var intencion = conv.contexts.get("luz-followup").parameters.intencion;
    switch(intencion){
        case "encender" : 
            respuesta = encenderLuz(habitacion, conv);
            conv.ask(respuesta);
            break;
        case "apagar" : 
            respuesta = apagarLuz(habitacion, conv); 
            conv.ask(respuesta);
            break;
        case "consultar estado dispositivo" : 
            return consultarEstadoLuz(habitacion, conv).then((respuesta) => {
                conv.ask(respuesta);
            });
    }
});

app.intent('consultar temperatura', (conv) => {
    return consultarTemperatura().then((temperatura) => {
        conv.ask("Hay " + temperatura + " grados centígrados de temperatura");
      });
});

app.intent('programar temperatura', (conv, {dispositivo, temperatura}) => {
    if(dispositivo == "calefaccion"){
        baseDatos.ref("/comun/calefaccion").update({"temperaturaProgramada": temperatura});
        conv.ask("La calefacción se ha programado a "+ temperatura + " grados");
    }
    else conv.ask("Solo se puede programar la temperatura de la calefacción");
});

app.intent('programar hora', (conv, {dispositivo, fecha}) => {
    if(dispositivo == "despertador"){
        var objetoFecha = new Date(fecha);
        var hora = objetoFecha.getHours() + 1;
        var minuto = objetoFecha.getMinutes();
        baseDatos.ref("/dormitorio/despertador").update({"horaProgramada": hora + ":" + minuto});
        conv.ask("La alarma del despertador se ha programado a las " + hora + ":" + minuto);
    }
    else conv.ask("Solo se puede programar la hora del despertador");
});

exports.fulfillmentDomotica = functions.https.onRequest(app);

exports.obtenerHoraActual = functions.https.onRequest((req, res) => {
    var fechaActual = new Date();
    var hora = fechaActual.getHours() + 2 //zona GMT+2
    var minuto = fechaActual.getMinutes();
    res.send(hora + ":" + minuto);
});

/*****************************************
            FUNCIONES AUXILIARES
 *****************************************/

 /*************************************
                CONTROL DE LUCES
  ****************************************/
function encenderLuz(habitacion, conv){
    if (habitacion){
        baseDatos.ref("/" + habitacion).update({"luz": true});
        if(habitacion == "salon") habitacion = "salón";
        return "La luz del " + habitacion + " se ha encendido";
    }
    else{
        conv.contexts.set('luz-followup', 1, {"intencion":conv.intent});
        return "¿En qué habitación quiere que encienda la luz?";
    }
}

function apagarLuz(habitacion, conv){
    if (habitacion){
        baseDatos.ref("/" + habitacion).update({"luz": false});
        if(habitacion == "salon") habitacion = "salón";
        return "La luz del " + habitacion + " se ha apagado";
    }
    else{
        conv.contexts.set('luz-followup', 1, {"intencion":conv.intent});
        return "¿En qué habitación quiere que apague la luz?";
    }
}

function consultarEstadoLuz(habitacion, conv){
    var estado;
    if (habitacion){
        return new Promise((resolve, reject) => {
            baseDatos.ref("/" + habitacion + "/luz").once("value", (datos) => {
                if (datos.val()) estado = "encendida";
                else estado = "apagada";
                if(habitacion == "salon") habitacion = "salón";
                resolve ("La luz del " + habitacion + " está " + estado);
            });
        });
    }
    else{
        conv.contexts.set('luz-followup', 1, {"intencion":conv.intent});
        return new Promise((resolve, reject) => {
            resolve ("¿De qué habitación quiere saber el estado de la luz?");
        });
    }
}

 /*************************************
        CONSULTA DE LA TEMPERATURA
  ****************************************/

function consultarTemperatura(){
    return new Promise((resolve, reject) => {
        baseDatos.ref("/comun/sensorTemperatura").once("value", (datos) => {
            resolve (datos.val());
        });
    });
}

 /*************************************
        CONTROL DE LA CALEFACCIÓN
  ****************************************/

function encenderCalefaccion(){
    baseDatos.ref("/comun/calefaccion").update({"estado": true});
        return "Calefacción encendida";
}

function apagarCalefaccion(){
    baseDatos.ref("/comun/calefaccion").update({"estado": false});
        return "Calefacción apagada";
}

function consultarEstadoCalefaccion(){
    return new Promise((resolve, reject) => {
        baseDatos.ref("/comun/calefaccion/estado").once("value", (datos) => {
            if(datos.val()){
                baseDatos.ref("/comun/calefaccion/temperaturaProgramada").once("value", (datos) => {
                    resolve ("La calefacción está encendida y programada a " + datos.val() + " grados");
                });
            }
            else resolve ("La calefacción está apagada");
        });
    });
}

 /*************************************
        CONTROL DEL DESPERTADOR
  ****************************************/

function activarAlarmaDespertador(){
    baseDatos.ref("/dormitorio/despertador").update({"estado": true});
    return "La alarma del despertador se ha activado";
}

function desactivarAlarmaDespertador(){
    baseDatos.ref("/dormitorio/despertador").update({"estado": false});
    return "La alarma del despertador se ha desactivado";
}

function consultarEstadoDespertador(){
    return new Promise((resolve, reject) => {
        baseDatos.ref("/dormitorio/despertador/estado").once("value", (datos) => {
            if(datos.val()){
                baseDatos.ref("/dormitorio/despertador/horaProgramada").once("value", (datos) => {
                    resolve ("La alarma del despertador sonará a las " + datos.val());
                });
            }
            else resolve ("La alarma del despertador está desactivada");
        });
    });
}

