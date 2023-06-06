const express = require("express");
const bodyParser = require("body-parser");

const app = express();
const PORT = 6666;
app.use(bodyParser.json());

app.post("/", (req, res) => {
  console.log(`Received POST request from ${req.ip} with body: ${req.body}`);
  res.send("{Received POST request}");
});

app.listen(PORT, "0.0.0.0", () => {
  console.log();
});
