import json

def func_prueba():
    print("Hola desde python :)")

def selectPreguntas():
    print("Seleccionar preguntas")
    with open('./json/preguntas.json') as json_file:  
        data = json.load(json_file)
        return(data['preguntas']['pregunta1']['enunciado'])
        #for p in data['preguntas']:
         #   return(p['pregunta1']['enunciado'])


def escribirJSON():
    person = {
        'first_name': "John",
        "isAlive": True,
        "age": 27,
        "address": {
            "streetAddress": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postalCode": "10021-3100"
        },
        "hasMortgage": None
    }
    with open('./json/usuarios.json', 'w') as f:  # writing JSON object
        json.dump(person, f)