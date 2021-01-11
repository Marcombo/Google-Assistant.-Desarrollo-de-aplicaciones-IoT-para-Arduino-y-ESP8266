'use strict';

const functions = require('firebase-functions');
const {dialogflow} = require('actions-on-google');

const app = dialogflow({debug: true});

app.intent('Default Welcome Intent', (conv) => {
  conv.ask("Hola, soy su asistente personal. ¿Usted cómo se llama?");
});

app.intent('obtener nombre', (conv, {nombre}) => {
  console.log("Su nombre es: " + nombre.name);
  conv.ask(nombre.name + ", ¿quieres jugar conmigo?");
});

app.intent('adivina el numero', (conv) => {
  conv.data.numeroOculto = undefined;
  conv.data.intentos = undefined;
  conv.ask("Dígame un número del 1 al 10");
});

app.intent('obtener número', (conv, {numero}) => {
  let numeroOculto, intentos;
  
  if(numero > 10 || numero < 1){
    conv.ask("Debe decir un número comprendido entre 1 y 10");
    return;
  }
  
  if (conv.data.numeroOculto == undefined){
    numeroOculto = Math.floor((Math.random() * 9) + 1);
    conv.data.numeroOculto = numeroOculto;
    conv.data.intentos = 3;
  }
  else numeroOculto = conv.data.numeroOculto;

  intentos = conv.data.intentos - 1;
  conv.data.intentos = intentos;
  if(numero == numeroOculto)  conv.close('Enhorabuena, ha acertado');
  else if(intentos ==0) conv.close('No le quedan más intentos. El número era el '+ numeroOculto);
  	   else if (numero > numeroOculto)  conv.ask('El número en el que he pensado es menor');
  		    else if (numero < numeroOculto)  conv.ask('El número en el que he pensado es mayor');
});

app.intent('sigue la secuencia', (conv) => {
  conv.data.secuenciaNumeros = generarSecuencia();
  conv.ask("Repita los siguientes números: "+ conv.data.secuenciaNumeros);
});

app.intent('obtener secuencia', (conv, {secuencia}) => {
  let secuenciaAsistente = conv.data.secuenciaNumeros;
  let digitosSecuencia = Math.ceil(secuenciaAsistente.length/2);
  let secuenciaUsuario = obtenerSecuencia(secuencia);

  if(secuenciaUsuario == secuenciaAsistente){
    let nuevoNumero = Math.floor(Math.random() * 9);
    secuenciaAsistente = secuenciaAsistente+" "+nuevoNumero;
    conv.data.secuenciaNumeros = secuenciaAsistente;
    conv.ask(secuenciaAsistente);
  }
  else if (digitosSecuencia == 5) conv.close("Para poder jugar debe ser capaz de repetir, al menos, una secuencia de 5 números"); 
       else conv.close("¡Enhorabuena! Ha llegado a repetir una secuencia de " + (digitosSecuencia - 1).toString() + " dígitos");
});

function generarSecuencia(){
  let secuencia = "";
  for(var i = 0; i < 5; i++){
    secuencia = secuencia + Math.floor((Math.random() * 9) + 1) + " ";
  }
  secuencia = secuencia.substring(0, secuencia.length - 1);
  return secuencia;
}

function obtenerSecuencia(secuencia){
  var secuenciaNumeros="";
  for (var i = 0; i < secuencia.length; i++) {
    var numero = secuencia[i].toString();
    for (var j = 0; j < numero.length; j++) {
      secuenciaNumeros = secuenciaNumeros+numero.charAt(j) + " ";
    }
  }
  secuenciaNumeros = secuenciaNumeros.substring(0, secuenciaNumeros.length - 1);
  return secuenciaNumeros;
} 

exports.dialogflowFirebaseFulfillment = functions.https.onRequest(app);
