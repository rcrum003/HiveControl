<h1><i class="fa fa-cubes"></i> Hive Components</h1>

<div class="alert alert-info">
    <strong>About Hive Components:</strong> Configure the physical components of your beehive to help HiveControl calculate accurate estimates for honey stores, brood patterns, and hive capacity.
</div>

<h2>What are Hive Components?</h2>
<p>Hive components represent the physical structure of your beehive. By accurately configuring your hive body, HiveControl can:</p>
<ul>
    <li>Calculate estimated honey stores based on weight changes</li>
    <li>Track capacity across different hive boxes</li>
    <li>Provide more accurate forecasts for hive growth</li>
    <li>Help you plan hive inspections and maintenance</li>
</ul>

<h2>Common Hive Components</h2>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-square-o"></i> Hive Boxes</h4>
            <p>Configure your hive boxes (supers) including:</p>
            <ul>
                <li>Deep boxes (9 5/8")</li>
                <li>Medium boxes (6 5/8")</li>
                <li>Shallow boxes (5 11/16")</li>
            </ul>
            <p><strong>Tip:</strong> Include the weight of the empty box to get accurate honey weight calculations.</p>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-th"></i> Frames</h4>
            <p>Track frame counts per box:</p>
            <ul>
                <li>Standard: 10 frames per box</li>
                <li>Some beekeepers use 8 or 9 frames</li>
                <li>Foundation vs. drawn comb weight differences</li>
            </ul>
        </div>
    </div>
</div>

<div class="row">
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-server"></i> Bottom Board & Cover</h4>
            <p>Include the weight of:</p>
            <ul>
                <li>Bottom board (screened or solid)</li>
                <li>Inner cover</li>
                <li>Outer cover/telescoping top</li>
            </ul>
        </div>
    </div>
    <div class="col-md-6">
        <div class="feature-box">
            <h4><i class="fa fa-level-up"></i> Queen Excluder</h4>
            <p>If you use a queen excluder, add its weight to get accurate calculations.</p>
        </div>
    </div>
</div>

<h2>Configuring Hive Components</h2>

<div class="panel panel-default">
    <div class="panel-heading">
        <strong>Step-by-Step Configuration</strong>
    </div>
    <div class="panel-body">
        <p><span class="step-number">1</span> Navigate to <strong>Settings > Hive Components</strong></p>
        <p><span class="step-number">2</span> Enter the details for each component of your hive</p>
        <p><span class="step-number">3</span> Weigh empty boxes and frames to get accurate base weights</p>
        <p><span class="step-number">4</span> Update the configuration when you add or remove boxes</p>
        <p><span class="step-number">5</span> Save your changes</p>
    </div>
</div>

<h2>Weight Calculations</h2>
<p>HiveControl uses your component configuration to calculate:</p>
<div class="code-block">
    <strong>Honey Stores = Total Hive Weight - (Equipment Weight + Bee Weight + Brood Weight)</strong>
</div>

<h3>Important Considerations</h3>
<ul>
    <li><strong>Seasonal Changes:</strong> Update weights when you add or remove supers</li>
    <li><strong>Moisture:</strong> Wooden equipment can gain weight in humid conditions</li>
    <li><strong>Bee Population:</strong> A strong hive can have 5-10 lbs of bees</li>
    <li><strong>Pollen Stores:</strong> Stored pollen adds weight but isn't honey</li>
</ul>

<h2>Best Practices</h2>
<div class="alert alert-info">
    <ul style="margin-bottom: 0;">
        <li>Weigh components when dry for most accurate baseline</li>
        <li>Record weights with and without frames</li>
        <li>Update configuration immediately after hive inspections when you modify the structure</li>
        <li>Keep a log of changes for reference</li>
        <li>Consider seasonal equipment rotations (screened vs solid bottom boards)</li>
    </ul>
</div>

<h2>Common Questions</h2>

<h3>How often should I update my hive component configuration?</h3>
<p>Update whenever you add or remove boxes, change equipment, or notice significant weight discrepancies in your data.</p>

<h3>What if I don't know the exact weight of my components?</h3>
<p>Use standard weights as estimates:</p>
<ul>
    <li>Deep box with 10 frames: ~20-25 lbs</li>
    <li>Medium box with 10 frames: ~15-20 lbs</li>
    <li>Shallow box with 10 frames: ~12-15 lbs</li>
    <li>Bottom board: ~5-8 lbs</li>
    <li>Covers: ~8-12 lbs combined</li>
</ul>

<h3>Does HiveControl track individual box weights?</h3>
<p>No, the scale measures total hive weight. The component configuration helps estimate how that weight is distributed.</p>
