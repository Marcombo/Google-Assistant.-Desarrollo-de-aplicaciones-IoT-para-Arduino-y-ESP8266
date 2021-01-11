"use strict"

var miNumero = 3;

var esPar = numero => {
    if(numero % 2) return false;
    else return true;
}

var esPrimo = numero => {
    for (let i = 2; i < numero; i++) {
      if ( numero % i == 0) return false;
    }
    return true;
}

var esParPrimo = (numero, funcion) => funcion(numero);

if(esParPrimo(miNumero, esPar)) console.log("el número " + miNumero + " es par");
else console.log("el número " + miNumero + " no es par");

if(esParPrimo(miNumero, esPrimo)) console.log("el número " + miNumero + " es primo");
else console.log("el número " + miNumero + " no es primo");