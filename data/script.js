function bouttonConnexion(event) {
    event.preventDefault(); // Empêche l'envoi classique du formulaire

    // Récupère les valeurs du formulaire
    const ssid = document.querySelector('input[name="wifi_ssid"]').value.trim();
    const wifiMdp = document.querySelector('input[name="wifi_mdp"]').value.trim();
    const mqttIp = document.querySelector('input[name="mqtt_ip"]').value.trim();
    const mqttPort = document.querySelector('input[name="mqtt_port"]').value.trim();
    const mqttUser = document.querySelector('input[name="mqtt_user"]').value.trim();
    const mqttMdp = document.querySelector('input[name="mqtt_mdp"]').value.trim();
    const univers = document.querySelector('input[name="univers"]').value.trim();

    if (!ssid) {
        showPopup("Le nom du réseau Wi-Fi est requis.");
        return;
    }
    if (!mqttIp) {
        showPopup("L'adresse IP du broker MQTT est requise.");
        return;
    }
    if (!univers) {
        showPopup("Le numéro d'univers est requis.");
        return;
    }

    // Construit l'URL de la requête
    const url = `/config?wifi_ssid=${encodeURIComponent(ssid)}&wifi_mdp=${encodeURIComponent(wifiMdp)}&mqtt_ip=${encodeURIComponent(mqttIp)}&mqtt_port=${encodeURIComponent(mqttPort)}&mqtt_user=${encodeURIComponent(mqttUser)}&mqtt_mdp=${encodeURIComponent(mqttMdp)}&univers=${encodeURIComponent(univers)}`;

    // Envoie la requête au serveur ESP32
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", url, true);

    xhttp.onload = function () {
        if (xhttp.status === 200) {
            showPopup("Configuration envoyée avec succès !", "success");
        } else {
            showPopup("Erreur lors de l'envoi des données.");
        }
    };

    xhttp.onerror = function () {
        showPopup("Erreur de connexion avec l'ESP32.");
    };

    xhttp.send();
}

function showPopup(message, type = "error") {
    const popup = document.getElementById("popupAlert");
    const popupText = document.getElementById("popupMessage");

    // Détermine la couleur selon le type
    let colorClass = (type === "success") ? "w3-green" : "w3-red";

    // Applique la couleur et le message
    popup.className = `w3-panel ${colorClass} w3-display-container w3-animate-opacity`;
    popupText.textContent = message;
    popup.style.display = "block";

    // Cache le popup après 4 secondes
    setTimeout(() => {
        popup.style.display = "none";
    }, 4000);
}