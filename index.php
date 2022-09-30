<?php

/**
 * Convert .map, .pak and .sav files from /input into .png in /output.
 */
function convertMapsToPngs($extractPath, $destinationPath)
{
    $imagesPaths = [];

    // Get all file names :
    $it = new RecursiveDirectoryIterator($extractPath, RecursiveDirectoryIterator::SKIP_DOTS);
    $files = new RecursiveIteratorIterator($it, RecursiveIteratorIterator::CHILD_FIRST);
    foreach ($files as $file) {
        if (!$file->isDir()) {
            // Get extension of file :
            $extension = substr($file->getFilename(), -3);
            if ($extension === 'map' || $extension === 'pak' || $extension === 'sav') {
                // Get png file path :
                $baseName = substr($file->getFilename(), 0, -4);
                $pngPath = $destinationPath . '/' . $baseName . '.png';

                // Convert file to png :
                $execReturn = exec(__DIR__ . '/cbmappers/zeusmapper "' . $file->getPathName() . '" "' . $pngPath . '"');
            }
        }
    }

    return $imagesPaths;
}

// Convert .map to .png :
convertMapsToPngs(__DIR__ . '/input', __DIR__ . '/output');
