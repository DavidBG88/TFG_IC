const express = require('express');
const mysql = require('mysql2');
const dayjs = require('dayjs');
const customParseFormat = require('dayjs/plugin/customParseFormat');
dayjs.extend(customParseFormat);

const app = express();
app.use(express.json());

// Configuración de la conexión a la base de datos
const connection = mysql.createConnection({
    host: 'X',
    user: 'X',
    password: 'X',
    database: 'X'
});

// Verificar conexión a la base de datos
connection.connect((err) => {
    if (err) {
        console.error('Error al conectar a la base de datos:', err);
    } else {
        console.log('Conexión exitosa a la base de datos MySQL');
    }
});

// Endpoint para comprobar si el servidor funciona
app.get('/', (req, res) => {
    res.send('Servidor funcionando');
});

// Endpoint para recibir los datos de sensores
app.post('/master_data', (req, res) => {
    const sensorData = req.body;

    console.log('Datos recibidos:', sensorData);  // Traza para verificar los datos recibidos

    // SQL para insertar los datos en la tabla de la base de datos
    const sql = `INSERT INTO master_data (timestamp, aht20_temp, aht20_hum, veml7700, ens160_aqi, ens160_eco2, ens160_tvoc, water_level, soil_moisture, fan1_rpm, fan2_rpm)
                 VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;

    const fixedTimestamp = dayjs(sensorData.timestamp, 'HH:mm:ss DD/MM/YYYY').format('YYYY-MM-DD HH:mm:ss');

    const values = [
        fixedTimestamp,
        sensorData.aht20_temp,
        sensorData.aht20_hum,
        sensorData.veml7700,
        sensorData.ens160_aqi,
        sensorData.ens160_eco2,
        sensorData.ens160_tvoc,
        sensorData.water_level,
        sensorData.soil_moisture,
        sensorData.fan1_rpm,
        sensorData.fan2_rpm
    ];

    connection.query(sql, values, (err, results) => {
        if (err) {
            console.error('Error al insertar los datos:', err);  // Traza de error
            res.status(500).send('Error al insertar los datos');
        } else {
            console.log('Datos insertados correctamente:', results);
            res.status(200).send('Datos insertados correctamente');
        }
    });
});

// Endpoint para recibir los datos de esclavos
app.post('/slave_data', (req, res) => {
    const sensorData = req.body;

    // Verificar de qué esclavo provienen los datos usando el campo `slave_id`
    if (sensorData.id === 1) {
        console.log('Datos de esclavo 1 recibidos:', sensorData);
    } else if (sensorData.id === 2) {
        console.log('Datos de esclavo 2 recibidos:', sensorData);
    } else {
        console.log('Datos de esclavo desconocido recibidos:', sensorData);
    }

    // SQL para insertar los datos en la tabla `slave_data`
    const sql = `INSERT INTO slave_data (slave_id, temperatura, humedad, timestamp)
                 VALUES (?, ?, ?, ?)`;

    const fixedTimestamp = dayjs(sensorData.timestamp, 'HH:mm:ss DD/MM/YYYY').format('YYYY-MM-DD HH:mm:ss');

    const values = [
        sensorData.id,
        sensorData.temperatura,
        sensorData.humedad,
        fixedTimestamp
    ];

    connection.query(sql, values, (err, results) => {
        if (err) {
            console.error(`Error al insertar los datos del esclavo ${sensorData.id}:`, err);
            res.status(500).send(`Error al insertar los datos del esclavo ${sensorData.id}`);
        } else {
            console.log(`Datos del esclavo ${sensorData.id} insertados correctamente:`, results);
            res.status(200).send(`Datos del esclavo ${sensorData.id} insertados correctamente`);
        }
    });
});

// Endpoint para recibir los datos meteorológicos de la API
app.post('/api_data', (req, res) => {
    const apiData = req.body;

    console.log('Datos meteorológicos recibidos:', apiData);

    // SQL para insertar los datos en la tabla api_data
    const sql = `INSERT INTO api_data (
        timestamp, temp, feels_like, temp_min, temp_max, pressure, hum,
        cloudiness, windSpeed, windDeg, precProb, rainVolume, snowVolume
    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`;

    const fixedTimestamp = dayjs(apiData.timestamp, 'HH:mm:ss DD/MM/YYYY').format('YYYY-MM-DD HH:mm:ss');

    const values = [
        fixedTimestamp,
        apiData.temp,
        apiData.feels_like,
        apiData.temp_min,
        apiData.temp_max,
        apiData.pressure,
        apiData.humidity,
        apiData.cloudiness,
        apiData.wind_speed,
        apiData.wind_deg,
        apiData.prec_prob,
        apiData.rain_volume,
        apiData.snow_volume
    ];

    connection.query(sql, values, (err, results) => {
        if (err) {
            console.error('Error al insertar los datos meteorológicos:', err);
            res.status(500).send('Error al insertar los datos meteorológicos');
        } else {
            console.log('Datos meteorológicos insertados correctamente:', results);
            res.status(200).send('Datos meteorológicos insertados correctamente');
        }
    });
});

// Iniciar el servidor en el puerto 4000 con la IP 0.0.0.0 para estar accesible en la red local
app.listen(4000, '0.0.0.0', () => {
    console.log('Servidor ejecutándose en http://192.168.X.X:4000');
});
