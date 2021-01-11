const functions = require('firebase-functions');
const firebase = require('firebase-admin');

const app = firebase.initializeApp();
const baseDatos = firebase.database();

exports.enciendeLed = functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01").update({"D0": true});
    response.send("Se ha asignado el valor true al GPIO0");
});

exports.apagaLed = functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01").update({"D0": false});
    response.send("Se ha asignado el valor false al GPIO0");
});

exports.estadoDispositivo= functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01/D2").once("value", (datos) => {
        response.send("El estado del dispositivo es: " + datos.val());
    });
});

exports.estadoNodos= functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01").once("value", (datos) => {
        var valorNodos = "El valor del nodo D0 es: " + datos.child("D0").val();
        valorNodos += "<br>" + "El valor del nodo D2 es: " + datos.child("D2").val()
        response.send(valorNodos);
    });
});

exports.crearNodoD1= functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01/D1").push();
    baseDatos.ref("/ESP-01").update({"D1":false});
    response.send("Se ha creado el nodo D1");
});

exports.borrarNodoD1= functions.https.onRequest((request, response) => {
    baseDatos.ref("/ESP-01/D1").remove();
    response.send("Se ha borrado el nodo D1");
});

