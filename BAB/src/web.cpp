

// void page_hotspot_string(String *string_html, settings_context settings) {
//     *string_html += "\n";
//     *string_html += "<!DOCTYPE html>";
//     *string_html += "<html>";
//     *string_html += "<head>";
//     *string_html += "<title>Formulaire de configuration</title>";
//     *string_html += "</head>";
//     *string_html += "<body>";
//     *string_html += "  <h2>Configuration du systeme</h2>";
//     *string_html += "  <form action='/' method='post'>";
//     *string_html += "    <label for='input1'>Ssid :</label>";
//     *string_html += "    <input type='text' id='input1' name='input1' value='"; *string_html += settings.wifi_ssid; *string_html += "'><br><br>";

//     *string_html += "    <label for='input2'>Mot de passe :</label>";
//     *string_html += "    <input type='text' id='input2' name='input2' value='"; *string_html += settings.wifi_pass; *string_html += "'><br><br>";

//     *string_html += "    <label for='seuilHaut'>Seuil haut :</label>";
//     *string_html += "    <input type='range' id='seuilHaut' onchange='seuil_value(this)' name='seuilHaut' min='0' max='255' value='"; *string_html += String(settings.seuil_1); *string_html += "'><br><br>";

//     *string_html += "    <label for='seuilBas'>Seuil bas :</label>";
//     *string_html += "    <input type='range' id='seuilBas' onchange='seuil_value(this)' name='seuilBas' min='0' max='255' value='"; *string_html += String(settings.seuil_2); *string_html += "'><br><br>";

//     *string_html += "    <label for='sensibilite'>Sensibilite :</label>";
//     *string_html += "    <input type='range' id='sensibilite' name='sensibilite' min='0' max='255' value='"; *string_html += String(settings.sensitivity); *string_html += "'><br><br>";
        
//     *string_html += "    <label for='luminosite'>Luminosite :</label>";
//     *string_html += "    <input type='range' id='luminosite' name='luminosite' min='0' max='255' value='"; *string_html += String(settings.brightness); *string_html += "'><br><br>";
    
//     *string_html += "    <label for='jauge'>Jauge (else smiley) :</label>";
//     *string_html += "    <input type='checkbox' id='jauge' name='jauge' "; *string_html += settings.mode_jauge_smiley ? "checked" : ""; *string_html += "><br><br>";

//     *string_html += "    <label for='wifi_type'>Connect to ssid :</label>";
//     *string_html += "    <input type='checkbox' id='connect_type' name='connect_type' "; *string_html += settings.connect_type ? "checked" : ""; *string_html += "><br><br>";
        
//     *string_html += "    <input type='submit' value='Envoyer'>";
//     *string_html += "  </form>";
//     *string_html += "</body>";
//     *string_html += " <script type='text/javascript'>"
//                     "function seuil_value(e){"
//                     "let seuilHaut = parseInt(document.getElementById('seuilHaut').value);"
//                     "let seuilBas = parseInt(document.getElementById('seuilBas').value);"
//                     "if ((e.id === 'seuilHaut') && (seuilHaut <= seuilBas)){"
//                     "    document.getElementById('seuilBas').value = document.getElementById('seuilHaut').value;"
//                     "}"
//                     "else if((e.id === 'seuilBas') && (seuilHaut <= seuilBas)){"
//                     "    document.getElementById('seuilHaut').value = document.getElementById('seuilBas').value;"
//                     "}"
//                     "}"
//                     "</script>";
//     *string_html += "</html>";
//     *string_html += "\n";
//     *string_html += "\n";
//     //Serial.println(*string_html);
// }
