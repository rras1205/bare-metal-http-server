param($port, $name)

$url = "http://localhost:$port/"
$count = 1000
echo "------------------------------------------------"
echo "Running Benchmark: $name (Port $port)"
echo "Firing $count requests..."

$time = Measure-Command {
    1..$count | ForEach-Object {
        $response = Invoke-WebRequest -Uri $url -UseBasicParsing
    }
}

echo "Total Time: $($time.TotalSeconds) seconds"
echo "Requests Per Second: $([math]::Round($count / $time.TotalSeconds, 2))"
echo "------------------------------------------------"